#pragma once

#include <stddef.h>
#include <stdlib.h>

namespace container {

template <typename T, size_t N> struct FixedSizeCircularBuffer;

template <typename T> struct CircularBuffer {
private:
  size_t capacity_;
  size_t head_ = 0, tail_ = 0;

  T* values_;

  CircularBuffer(size_t capacity, T* values)
      : capacity_(capacity), values_(values) {}

  template <typename, size_t> friend struct FixedSizeCircularBuffer;

  size_t nextIndex(size_t i) { return (i + 1) % capacity_; }

public:
  CircularBuffer() = delete;

  bool empty() { return head_ == tail_; }
  bool full() { return nextIndex(tail_) == head_; }

  void push_back(T value) {
    if (full()) {
      abort();
    }

    values_[tail_] = value;
    tail_ = nextIndex(tail_);
  }

  T& front() {
    if (empty()) {
      abort();
    }

    return values_[head_];
  }

  void pop_front() {
    if (empty()) {
      abort();
    }

    head_ = nextIndex(head_);
  }
};

template <typename T, size_t N>
struct FixedSizeCircularBuffer : public CircularBuffer<T> {
  FixedSizeCircularBuffer() : CircularBuffer<T>(N, values_) {}

private:
  T values_[N];
};

}; // namespace container
