#ifndef LAZY_PTR_HPP
#define LAZY_PTR_HPP

#include <functional>
#include <memory>
#include <utility>

namespace LazyPtr {
using namespace std;

namespace detail {
template <typename T> class lazy_task {
  mutable function<T()> task;
  mutable shared_ptr<T> valueptr;

public:
  bool invariant() const noexcept { return bool(task) ^ bool(valueptr); }

  lazy_task() = delete;
  lazy_task(const lazy_task &) = delete;
  lazy_task(lazy_task &&) = delete;
  lazy_task &operator=(const lazy_task &) = delete;
  lazy_task &operator=(lazy_task &&) = delete;

  lazy_task(const function<T()> &task) : task(task) {}
  lazy_task(function<T()> &&task) noexcept : task(move(task)) {}

  bool ready() const { return bool(valueptr); }
  shared_ptr<T> run() const {
    if (!valueptr) {
      if (!task)
        throw logic_error("Recursive access to lazy_ptr");
      function<T()> task1;
      using std::swap;
      swap(task1, task);
      valueptr = make_shared<T>(task1());
    }
    return valueptr;
  }
};
}

template <typename T> class lazy_ptr {
  typedef detail::lazy_task<T> task_t;
  mutable shared_ptr<task_t> task;
  mutable shared_ptr<T> ptr;

  void run() const {
    if (task) {
      ptr = task->run();
      task = nullptr;
    }
  }

public:
  typedef T element_type;

  bool invariant() const noexcept {
    return !(bool(task) & bool(ptr)) && (!task || task->invariant());
  }

  lazy_ptr() noexcept = default;

  lazy_ptr(const lazy_ptr &p) noexcept = default;
  lazy_ptr(lazy_ptr &&p) noexcept = default;
  lazy_ptr &operator=(const lazy_ptr &p) noexcept = default;
  lazy_ptr &operator=(lazy_ptr &&p) noexcept = default;

  lazy_ptr(const function<T()> &f) : task(make_shared<task_t>(f)) {}
  lazy_ptr(function<T()> &&f) : task(make_shared<task_t>(move(f))) {}
  lazy_ptr(const shared_ptr<T> &p) noexcept : ptr(p) {}
  lazy_ptr(shared_ptr<T> &&p) noexcept : ptr(move(p)) {}
  lazy_ptr(unique_ptr<T> &&p) : ptr(shared_ptr<T>(move(p))) {}

  void reset() noexcept {
    task = nullptr;
    ptr = nullptr;
  }
  void swap(lazy_ptr &p) noexcept {
    using std::swap;
    swap(task, p.task);
    swap(ptr, p.ptr);
  }

  operator bool() const noexcept { return bool(task) | bool(ptr); }
  bool ready() const noexcept { return !task || task->ready(); }
  shared_ptr<T> shared() const {
    run();
    return ptr;
  }
  T *get() const {
    run();
    return ptr.get();
  }
  T &operator*() const { return *get(); }
  T *operator->() const { return get(); }
};

template <typename T> void swap(lazy_ptr<T> &x, lazy_ptr<T> &y) noexcept {
  x.swap(y);
}

template <typename T, typename... Args> lazy_ptr<T> make_lazy(Args &&... args) {
  return lazy_ptr<T>(function<T()>([=] { return T(args...); }));
}

template <typename T> lazy_ptr<T> make_ready_lazy(T &&x) {
  return lazy_ptr<T>(make_shared<T>(forward<T>(x)));
}

template <typename F, typename... Args,
          typename R = typename result_of<F(Args...)>::type,
          typename T = typename decay<R>::type>
lazy_ptr<T> lazy(F &&f, Args &&... args) {
  return lazy_ptr<T>(function<T()>([=] { return f(args...); }));
}
}

#endif // LAZY_PTR_HPP
