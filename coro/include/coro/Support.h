#pragma once

namespace std {
namespace experimental {

template <typename _Promise = void> class coroutine_handle;

template <> class coroutine_handle<void> {
public:
  coroutine_handle() : __handle_(nullptr) {}
  coroutine_handle(nullptr_t) : __handle_(nullptr) {}

  coroutine_handle& operator=(nullptr_t) {
    __handle_ = nullptr;
    return *this;
  }

  // Conversion to/from address

  void* address() const { return __handle_; }

  static coroutine_handle from_address(void* __addr) {
    coroutine_handle __tmp;
    __tmp.__handle_ = __addr;
    return __tmp;
  }

  static coroutine_handle from_address(nullptr_t) {
    return coroutine_handle(nullptr);
  }

  template <class _Tp, bool _CallIsValid = false>
  static coroutine_handle from_address(_Tp*) {
    static_assert(_CallIsValid,
                  "coroutine_handle<void>::from_address cannot be called with "
                  "non-void pointers");
  }

  explicit operator bool() const { return __handle_; }

  void operator()() { resume(); }

  void resume() { __builtin_coro_resume(__handle_); }
  void destroy() { __builtin_coro_destroy(__handle_); }
  bool done() const { return __builtin_coro_done(__handle_); }

private:
  bool __is_suspended() const {
    // FIXME actually implement a check for if the coro is suspended.
    return __handle_;
  }

  template <class _PromiseT> friend class coroutine_handle;
  void* __handle_;
};

template <class T> T* addressof(T& arg) {
  return reinterpret_cast<T*>(
      &const_cast<char&>(reinterpret_cast<const volatile char&>(arg)));
}

template <typename _Promise>
class coroutine_handle : public coroutine_handle<> {
  using _Base = coroutine_handle<>;

public:
  // 18.11.2.1 construct/reset
  using coroutine_handle<>::coroutine_handle;

  coroutine_handle() : _Base() {}
  coroutine_handle(nullptr_t) : _Base(nullptr) {}

  coroutine_handle& operator=(nullptr_t) {
    _Base::operator=(nullptr);
    return *this;
  }

  _Promise& promise() const {
    return *static_cast<_Promise*>(
        __builtin_coro_promise(this->__handle_, __alignof(_Promise), false));
  }

public:
  static coroutine_handle from_address(void* __addr) {
    coroutine_handle __tmp;
    __tmp.__handle_ = __addr;
    return __tmp;
  }

  // NOTE: this overload isn't required by the standard but is needed so
  // the deleted _Promise* overload doesn't make from_address(nullptr)
  // ambiguous.
  // FIXME: should from_address work with nullptr?

  static coroutine_handle from_address(nullptr_t) {
    return coroutine_handle(nullptr);
  }

  template <class _Tp, bool _CallIsValid = false>
  static coroutine_handle from_address(_Tp*) {
    static_assert(
        _CallIsValid,
        "coroutine_handle<promise_type>::from_address cannot be called with "
        "non-void pointers");
  }

  template <bool _CallIsValid = false>
  static coroutine_handle from_address(_Promise*) {
    static_assert(
        _CallIsValid,
        "coroutine_handle<promise_type>::from_address cannot be used with "
        "pointers to the coroutine's promise type; use 'from_promise' instead");
  }

  static coroutine_handle from_promise(_Promise& __promise) {
    coroutine_handle __tmp;
    __tmp.__handle_ = __builtin_coro_promise(
        addressof(const_cast<_Promise&>(__promise)), __alignof(_Promise), true);
    return __tmp;
  }
};

} // namespace experimental
} // namespace std

namespace std {
namespace experimental {

template <typename R, typename... Args> struct coroutine_traits {
  using promise_type = typename R::promise_type;
};

struct suspend_never {
  bool await_ready() const noexcept { return true; }
  void await_suspend(coroutine_handle<>) const noexcept {}
  void await_resume() const noexcept {}
};

struct suspend_always {
  bool await_ready() const noexcept { return false; }
  void await_suspend(coroutine_handle<>) const noexcept {}
  void await_resume() const noexcept {}
};

} // namespace experimental
} // namespace std
