#pragma once

template <typename T, int bufferSize> class RingBuffer {
private:
  T data_[bufferSize];
  int head_;
  int tail_;

public:
  RingBuffer() : head_(0), tail_(0) {}

  RingBuffer(RingBuffer &&move) = delete;
  RingBuffer(RingBuffer const &copy) = delete;

  bool push(T value) volatile {
    if ((tail_ + 1) % bufferSize == head_) {
      // We're full
      return false;
    }

    data_[tail_] = value;
    tail_ = (tail_ + 1) % bufferSize;
    return true;
  }

  bool pop(T &output) volatile {
    if (head_ == tail_) {
      // We're empty
      return false;
    }

    output = data_[head_];
    head_ = (head_ + 1) % bufferSize;
    return true;
  }

  void clear() volatile { head_ = tail_ = 0; }

  int size() volatile { return (tail_ + bufferSize - head_) % bufferSize; }

  bool empty() volatile { return head_ == tail_; }

  T operator[](int index) volatile { return data_[head_ + index % bufferSize]; }
};
