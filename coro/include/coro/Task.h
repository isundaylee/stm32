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

    bool await_ready() { return !cont_; }

    template <typename P>
    auto await_suspend(std::experimental::coroutine_handle<P>) {
      return cont_;
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

private:
  std::experimental::coroutine_handle<> cont_ = nullptr;
};

template <> struct TaskPromise<void> : public BaseTaskPromise {
  auto get_return_object();
  auto return_void() {}
  void get() {}
};

template <typename T> struct TaskPromise : public BaseTaskPromise {
  auto get_return_object();
  auto return_value(T value) { value_ = value; }
  T get() { return value_; }

private:
  T value_;
};

} // namespace detail

template <typename T = void> struct Task {
public:
  using promise_type = typename detail::TaskPromise<T>;

  bool await_ready() { return false; }
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

auto TaskPromise<void>::get_return_object() {
  return Task<void>(
      std::experimental::coroutine_handle<TaskPromise<void>>::from_promise(
          *this));
}

template <typename T> auto TaskPromise<T>::get_return_object() {
  return Task<T>(
      std::experimental::coroutine_handle<TaskPromise<T>>::from_promise(*this));
}

} // namespace detail
