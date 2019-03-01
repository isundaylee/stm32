#pragma once

#include <stdlib.h>

#include "coro/Support.h"

template <typename T> struct Generator {
public:
  struct promise_type {
  public:
    auto initial_suspend() { return std::experimental::suspend_always{}; }
    auto final_suspend() { return std::experimental::suspend_always{}; }

    auto get_return_object() {
      return Generator<T>(handle_t::from_promise(*this));
    }
    void return_void() {}
    auto yield_value(T value) {
      value_ = value;
      return std::experimental::suspend_always{};
    }
    void unhandled_exception() { abort(); }

    friend struct Generator<T>;

  private:
    T value_;
  };

private:
  using handle_t = std::experimental::coroutine_handle<promise_type>;

  handle_t coro_;

public:
  Generator(handle_t coro) : coro_(coro) {}
  ~Generator() { coro_.destroy(); }

  // Move-only
  Generator(Generator<T> const&) = delete;
  Generator& operator=(Generator<T> const&) = delete;
  Generator(Generator<T>&&) = default;
  Generator& operator=(Generator<T>&&) = default;

  T& get() { return coro_.promise().value_; }

public:
  struct Iterator {
    Iterator(Generator<T>& generator, bool done)
        : generator_(generator), done_(done) {
      if (!done) {
        ++(*this);
      }
    }

    Iterator& operator++() {
      generator_.coro_.resume();
      done_ = generator_.coro_.done();
      return *this;
    }

    bool operator!=(Iterator const& rhs) const { return rhs.done_ != done_; }

    T& operator*() { return generator_.get(); }

  private:
    Generator<T>& generator_;
    bool done_;
  };

public:
  Iterator begin() { return Iterator{*this, false}; }
  Iterator end() { return Iterator{*this, true}; }
};
