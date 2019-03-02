#pragma once

#include "coro/Support.h"

#include <stdlib.h>

namespace detail {

template <typename T> struct TaskPromise;

struct BaseTaskPromise {
public:
  struct FinalSuspendAwaiter {
  public:
    FinalSuspendAwaiter(std::experimental::coroutine_handle<> cont)
        : cont_(cont) {}

    bool await_ready() { return false; }

    template <typename P>
    auto await_suspend(std::experimental::coroutine_handle<P>) {
      if (!!cont_) {
        cont_.resume();
      }
    }

    void await_resume() {}

  private:
    std::experimental::coroutine_handle<> cont_;
  };

  auto initial_suspend() { return std::experimental::suspend_never{}; }
  auto final_suspend() { return FinalSuspendAwaiter{cont_}; }
  void unhandled_exception() { abort(); }

  void setContinuation(std::experimental::coroutine_handle<> handle) {
    cont_ = handle;
  }

  bool isReady() { return ready_; }
  void markReady() { ready_ = true; }

private:
  std::experimental::coroutine_handle<> cont_ = nullptr;
  bool ready_ = false;
};

template <> struct TaskPromise<void> : public BaseTaskPromise {
  auto get_return_object();
  auto return_void() { markReady(); }
  void get() {}
};

template <typename T> struct TaskPromise : public BaseTaskPromise {
  auto get_return_object();
  auto return_value(T value) {
    value_ = value;
    markReady();
  }
  T get() { return value_; }

private:
  T value_;
};

} // namespace detail

template <typename T = void> struct Task {
public:
  using promise_type = typename detail::TaskPromise<T>;

  bool await_ready() { return coro_.promise().isReady(); }
  void await_suspend(std::experimental::coroutine_handle<> awaiter) {
    coro_.promise().setContinuation(awaiter);
  }
  auto await_resume() { return coro_.promise().get(); }

private:
  using Handle = std::experimental::coroutine_handle<promise_type>;

  Handle coro_;

public:
  Task(Handle coro) : coro_(coro) {}
  ~Task() { coro_.destroy(); }

  Task(Task const&) = delete;
  Task& operator=(Task const&) = delete;
  Task(Task&&) = default;
  Task& operator=(Task&&) = default;
};

namespace detail {

inline auto TaskPromise<void>::get_return_object() {
  return Task<void>(
      std::experimental::coroutine_handle<TaskPromise<void>>::from_promise(
          *this));
}

template <typename T> auto TaskPromise<T>::get_return_object() {
  return Task<T>(
      std::experimental::coroutine_handle<TaskPromise<T>>::from_promise(*this));
}

} // namespace detail
