#pragma once

#include <stddef.h>
#include <stdint.h>

template <typename T, size_t n> class FreeListBuffer {
private:
  struct Node {
    T object;
    Node* nextFree = nullptr;
  };

  Node nodes_[n] = {0};
  Node* freeHead_ = nullptr;

public:
  FreeListBuffer() {
    nodes_[0].nextFree = nullptr;

    // Free for all! :)
    for (size_t i = 0; i < n; i++) {
      free(&nodes_[i].object);
    }
  };

  T* allocate() {
    if (!freeHead_) {
      return nullptr;
    }

    Node* allocated = freeHead_;

    freeHead_ = freeHead_->nextFree;
    return &allocated->object;
  }

  void free(T* object) {
    Node* node = reinterpret_cast<Node*>(reinterpret_cast<uintptr_t>(object) -
                                         offsetof(Node, object));

    node->nextFree = freeHead_;
    freeHead_ = node;
  }
};
