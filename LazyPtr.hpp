#ifndef LAZY_PTR_HPP
#define LAZY_PTR_HPP

#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <utility>

namespace LazyPtr {
using namespace std;

class flag {
  bool value;

public:
  constexpr flag(bool value = false) noexcept : value(value) {}
  constexpr flag(const flag &other) noexcept : value(other.value) {}
  flag &operator=(const flag &other) noexcept {
    value = other.value;
    return *this;
  }
  void reset() noexcept { value = false; }
  void swap(flag &other) noexcept { std::swap(value, other.value); }
  constexpr operator bool() const noexcept { return value; }
  flag operator++(int)noexcept {
    auto old = *this;
    value = true;
    return old;
  }
  friend constexpr bool operator==(flag x, flag y) noexcept {
    return x.value == y.value;
  }
  friend constexpr bool operator!=(flag x, flag y) noexcept {
    return x.value != y.value;
  }
  friend constexpr bool operator<(flag x, flag y) noexcept {
    return x.value < y.value;
  }
  friend constexpr bool operator<=(flag x, flag y) noexcept {
    return x.value <= y.value;
  }
  friend constexpr bool operator>(flag x, flag y) noexcept {
    return x.value > y.value;
  }
  friend constexpr bool operator>=(flag x, flag y) noexcept {
    return x.value >= y.value;
  }
  friend ostream &operator<<(ostream &os, flag f) { return os << f.value; }
  friend void swap(flag &x, flag &y) noexcept { x.swap(y); }
};

namespace detail {
template <typename T, typename R = typename decay<T>::type>
future<R> make_ready_future(T &&value) {
  promise<R> p;
  p.set_value(forward<T>(value));
  return p.get_future();
}
}

// This wrapper looks like a shared_ptr, but behaves like a shared_future

template <typename T> class lazy_ptr;
template <typename T> class lazy_weak_ptr;

template <typename T> class lazy_ptr {
  shared_future<shared_ptr<T>> ftr;

  // Note: A null pointer can be represented either by an invalid
  // future, or by a valid future containing a null pointer.

  friend class lazy_weak_ptr<T>;

public:
  typedef T element_type;

  constexpr lazy_ptr() noexcept {}
  constexpr lazy_ptr(nullptr_t) noexcept {}

  lazy_ptr(const lazy_ptr &p) noexcept : ftr(p.ftr) {}
  lazy_ptr(lazy_ptr &&p) noexcept : ftr(move(p.ftr)) {}

  // TODO: This should be private
  lazy_ptr(const shared_future<shared_ptr<T>> &f) noexcept : ftr(f) {}
  lazy_ptr(shared_future<shared_ptr<T>> &&f) noexcept : ftr(move(f)) {}
  lazy_ptr(future<shared_ptr<T>> &&p) noexcept : ftr(move(p)) {}

  lazy_ptr(const shared_ptr<T> &p) noexcept {
    if (p)
      ftr = detail::make_ready_future(p);
  }
  lazy_ptr(shared_ptr<T> &&p) noexcept {
    if (p)
      ftr = detail::make_ready_future(move(p));
  }
  lazy_ptr(unique_ptr<T> &&p) noexcept {
    if (p)
      ftr = detail::make_ready_future(make_shared<T>(move(*p)));
  }

  lazy_ptr &operator=(const lazy_ptr &p) = default;
  lazy_ptr &operator=(lazy_ptr &&p) = default;

  void reset() noexcept { ftr = {}; }
  void swap(lazy_ptr &p) noexcept { swap(ftr, p.ftr); }

  shared_ptr<T> shared() const {
    if (!ftr.valid())
      return {};
    return ftr.get();
  }
  T *get() const {
    if (!ftr.valid())
      return {};
    return ftr.get().get();
  }
  T &operator*() const { return *get(); }
  T *operator->() const { return get(); }
  long use_count() const noexcept {
    if (!ftr.valid())
      return 0;
    return ftr.get().use_count();
  }
  bool unique() const noexcept {
    if (!ftr.valid())
      return false;
    return ftr.get().unique();
  }
  explicit operator bool() const noexcept {
    if (!ftr.valid())
      return false;
    return bool(ftr.get());
  }

  // Functions from shared_futre
  void wait() const {
    if (!ftr.valid())
      return;
    ftr.wait();
  }

  // Operators
  // friend bool operator==(const lazy_ptr &p, const lazy_ptr &q) noexcept {
  //   return p.ftr == q.ftr;
  // }
  // friend bool operator!=(const lazy_ptr &p, const lazy_ptr &q) noexcept {
  //   return p.ftr != q.ftr;
  // }
  // friend bool operator<(const lazy_ptr &p, const lazy_ptr &q) noexcept {
  //   return p.ftr < q.ftr;
  // }
  // friend bool operator<=(const lazy_ptr &p, const lazy_ptr &q) noexcept {
  //   return p.ft < q.ftr;
  // }
  // friend bool operator>(const lazy_ptr &p, const lazy_ptr &q) noexcept {
  //   return p.ftr > q.ftr;
  // }
  // friend bool operator>=(const lazy_ptr &p, const lazy_ptr &q) noexcept {
  //   return p.ft > q.ftr;
  // }

  // friend ostream &operator<<(ostream &os, const lazy_ptr &p) {
  //   return os << "lazy_ptr{" << p.ftr << "}";
  // }

  // std::size_t hash() const noexcept {
  //   std::hash<shared_future<T>> hash1;
  //   return hash1(ftr) ^ std::size_t(0x6598838dc7306a6eULL);
  // }
};

template <typename T, typename... Args> lazy_ptr<T> make_lazy(Args &&... args) {
  return lazy_ptr<T>(
      async(launch::deferred, [=] { return make_shared<T>(args...); }));
}

template <typename T> void swap(lazy_ptr<T> &p, lazy_ptr<T> &q) noexcept {
  p.swap(q);
}

// Functions for shared_future

template <typename T, typename R = typename decay<T>::type>
lazy_ptr<R> make_ready_lazy(T &&x) {
  return lazy_ptr<R>(detail::make_ready_future(make_shared<R>(forward<T>(x))));
}

namespace detail {
template <typename T> struct is_shared_ptr : false_type {};
template <typename T> struct is_shared_ptr<shared_ptr<T>> : true_type {};
}

template <typename F, typename... Args,
          typename R = typename result_of<F(Args...)>::type,
          typename enable_if<detail::is_shared_ptr<R>::value>::type * = nullptr,
          typename T = typename decay<typename R::element_type>::type>
lazy_ptr<T> lazy_from_shared(F &&f, Args &&... args) {
  return lazy_ptr<T>(async(launch::deferred, f, args...));
}

template <typename F, typename... Args,
          typename R = typename result_of<F(Args...)>::type,
          typename T = typename decay<R>::type>
lazy_ptr<T> lazy(F &&f, Args &&... args) {
  return lazy_ptr<T>(
      async(launch::deferred, [=] { return make_shared<T>(f(args...)); }));
}
}

namespace std {
// template <typename T> struct hash<LazyPtr::lazy_ptr<T>> {
//   std::size_t operator()(const LazyPtr::lazy_ptr<T> &p) const noexcept {
//     return p.hash();
//   }
// };
}

namespace LazyPtr {

template <typename T> class lazy_weak_ptr {
  shared_future<weak_ptr<T>> ftr;

public:
  typedef T element_type;

  constexpr lazy_weak_ptr() noexcept {}

  lazy_weak_ptr(const lazy_weak_ptr &p) noexcept : ftr(p.ftr) {}
  lazy_weak_ptr(const lazy_ptr<T> &p) {
    const auto &f = p.ftr;
    if (f.valid()) {
      ftr = async(launch::deferred,
                  [](const shared_future<shared_ptr<T>> &f) {
                    return weak_ptr<T>(f.get());
                  },
                  f);
    }
  }
  lazy_weak_ptr(lazy_weak_ptr &&p) noexcept : ftr(move(p.ftr)) {}

  lazy_weak_ptr &operator=(const lazy_weak_ptr &p) = default;
  lazy_weak_ptr &operator=(const lazy_ptr<T> &p) {
    return *this = lazy_weak_ptr(p);
  }
  lazy_weak_ptr &operator=(lazy_weak_ptr &&p) = default;

  void reset() { ftr = {}; }
  void swap(lazy_weak_ptr &p) noexcept { swap(ftr, p.ftr); }

  weak_ptr<T> weak() const {
    if (!ftr.valid())
      return {};
    return ftr.get();
  }
  long use_count() const noexcept {
    if (!ftr.valid())
      return 0;
    return ftr.get().use_count();
  }
  bool expired() const noexcept { return ftr.get().expired(); }
  lazy_ptr<T> lock() const {
    if (!ftr.valid())
      return {};
    return async(
        launch::deferred,
        [](const shared_future<weak_ptr<T>> &f) { return f.get().lock(); },
        ftr);
  }
};

template <typename T>
void swap(lazy_weak_ptr<T> &p, lazy_weak_ptr<T> &q) noexcept {
  p.swap(q);
}
}

#endif // LAZY_PTR_HPP
