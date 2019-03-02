#pragma once

#include "coro/Support.h"

template <size_t TaskQueueSize, size_t WaitAreaSize> struct FixedSizeScheduler;

struct Scheduler {
  Scheduler() = delete;

  Scheduler(Scheduler const&) = delete;
  Scheduler& operator=(Scheduler const&) = delete;
  Scheduler(Scheduler&&) = default;
  Scheduler& operator=(Scheduler&&) = default;

  // Returns whether we're done.
  bool runOnce() {
    bool anyWaiting = false;

    for (size_t i = 0; i < waitAreaSize_; i++) {
      if (!waitArea_[i].occupied) {
        continue;
      }

      if (!waitArea_[i].coro) {
        continue;
      }

      if (waitArea_[i].ready) {
        enqueueTask(waitArea_[i].coro);
        waitArea_[i].coro = nullptr;
        waitArea_[i].occupied = false;
        waitArea_[i].ready = false;
      } else {
        anyWaiting = true;
      }
    }

    if (taskHead_ != taskTail_) {
      auto task = tasks_[taskHead_];
      taskHead_ = (taskHead_ + 1) % taskQueueSize_;
      task.resume();
      return false;
    }

    return !anyWaiting;
  }

  void run() {
    while (!runOnce()) {
    }
  }

public:
  // schedule implementation

  struct ScheduleAwaiter {
  public:
    ScheduleAwaiter(Scheduler& sched) : sched_(sched) {}

    bool await_ready() { return false; }
    void await_suspend(std::experimental::coroutine_handle<> handle) {
      sched_.enqueueTask(handle);
    }
    void await_resume() {}

  private:
    Scheduler& sched_;
  };

  struct WaitToken {
    WaitToken() : token_(-1) {}

    void* toContext() { return reinterpret_cast<void*>(token_); }
    static WaitToken fromContext(void* context) {
      return WaitToken{reinterpret_cast<size_t>(context)};
    }

  private:
    WaitToken(size_t token) : token_(token) {}
    size_t token_;

  public:
    friend struct WaitUntilAwaiter;
    friend struct Scheduler;
  };

  auto schedule() { return ScheduleAwaiter{*this}; }

public:
  // waitUntil implementation

  struct WaitUntilAwaiter {
  public:
    WaitUntilAwaiter(Scheduler& sched, WaitToken token)
        : sched_(sched), token_(token) {}

    bool await_ready() { return false; }
    void await_suspend(std::experimental::coroutine_handle<> handle) {
      sched_.waitArea_[token_.token_].coro = handle;
      sched_.waitArea_[token_.token_].occupied = true;
    }
    void await_resume() {}

  private:
    Scheduler& sched_;
    WaitToken token_;
  };

  auto allocateWaitToken() {
    for (size_t i = 0; i < waitAreaSize_; i++) {
      if (!waitArea_[i].occupied) {
        waitArea_[i].occupied = true;
        return WaitToken{i};
      }
    }

    // We're full!
    abort();
  }

  void postCompletion(WaitToken token) { waitArea_[token.token_].ready = true; }

  auto waitUntil(WaitToken token) { return WaitUntilAwaiter{*this, token}; }

private:
  // Task queue
  size_t taskHead_ = 0, taskTail_ = 0, taskQueueSize_;
  std::experimental::coroutine_handle<>* tasks_;

  // Wait area
  struct WaitEntry {
    bool occupied = false;
    volatile bool ready = false;
    std::experimental::coroutine_handle<> coro = nullptr;
  };

  size_t waitAreaSize_;
  WaitEntry* waitArea_;

  Scheduler(size_t taskQueueSize, std::experimental::coroutine_handle<>* tasks,
            size_t waitAreaSize, WaitEntry* waitArea)
      : taskQueueSize_(taskQueueSize), tasks_(tasks),
        waitAreaSize_(waitAreaSize), waitArea_(waitArea) {}

  void enqueueTask(std::experimental::coroutine_handle<> coro) {
    auto newTail = (taskTail_ + 1) % taskQueueSize_;

    if (newTail == taskHead_) {
      // Full
      abort();
    }

    tasks_[taskTail_] = coro;
    taskTail_ = newTail;
  }

  template <size_t TaskQueueSize, size_t WaitAreaSize>
  friend struct FixedSizeScheduler;
};

template <size_t TaskQueueSize, size_t WaitAreaSize>
struct FixedSizeScheduler : public Scheduler {
  FixedSizeScheduler()
      : Scheduler(TaskQueueSize, tasks_, WaitAreaSize, waitArea_) {}

private:
  std::experimental::coroutine_handle<> tasks_[TaskQueueSize];
  Scheduler::WaitEntry waitArea_[WaitAreaSize];
};
