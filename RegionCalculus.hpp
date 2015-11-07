#ifndef REGIONCALCULUS_HPP
#define REGIONCALCULUS_HPP

#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <limits>
#include <memory>
#include <vector>

#define REGIONCALCULUS_DEBUG 1

namespace RegionCalculus {

using std::abs;
using std::array;
using std::min;
using std::max;
using std::ostream;
using std::unique_ptr;
using std::vector;

////////////////////////////////////////////////////////////////////////////////
// Point
////////////////////////////////////////////////////////////////////////////////

namespace detail {
template <typename T> struct largeint { typedef T type; };
template <> struct largeint<short> { typedef long long type; };
template <> struct largeint<int> { typedef long long type; };
template <> struct largeint<long> { typedef long long type; };
}

template <typename T, int D> struct point {
  array<T, D> elt;
  point() {
    for (int d = 0; d < D; ++d)
      elt[d] = T(0);
  }
  point(const array<T, D> &p) : elt(p) {}
  point(const vector<T> &p) {
    assert(p.size() == D);
    for (int d = 0; d < D; ++d)
      elt[d] = p[d];
  }
  point(const point &p) = default;
  point(point &&p) = default;
  explicit point(const T &x) {
    for (int d = 0; d < D; ++d)
      elt[d] = x;
  }
  template <typename U> explicit point(const point<U, D> &p) {
    for (int d = 0; d < D; ++d)
      elt[d] = T(p.elt[d]);
  }
  operator vector<T>() const {
    vector<T> r(D);
    for (int d = 0; d < D; ++d)
      r[d] = elt[d];
    return r;
  }
  point &operator=(const point &p) = default;
  point &operator=(point &&p) = default;

  // Unary operators
  point operator+() const {
    point r;
    for (int d = 0; d < D; ++d)
      r.elt[d] = +elt[d];
    return r;
  }
  point operator-() const {
    point r;
    for (int d = 0; d < D; ++d)
      r.elt[d] = -elt[d];
    return r;
  }
  point operator~() const {
    point r;
    for (int d = 0; d < D; ++d)
      r.elt[d] = ~elt[d];
    return r;
  }
  point<bool, D> operator!() const {
    point<bool, D> r;
    for (int d = 0; d < D; ++d)
      r.elt[d] = !elt[d];
    return r;
  }

  // Assignment operators
  point &operator+=(const point &p) {
    for (int d = 0; d < D; ++d)
      elt[d] += p.elt[d];
    return *this;
  }
  point &operator-=(const point &p) {
    for (int d = 0; d < D; ++d)
      elt[d] -= p.elt[d];
    return *this;
  }
  point &operator*=(const point &p) {
    for (int d = 0; d < D; ++d)
      elt[d] *= p.elt[d];
    return *this;
  }
  point &operator&=(const point &p) {
    for (int d = 0; d < D; ++d)
      elt[d] &= p.elt[d];
    return *this;
  }
  point &operator|=(const point &p) {
    for (int d = 0; d < D; ++d)
      elt[d] |= p.elt[d];
    return *this;
  }
  point &operator^=(const point &p) {
    for (int d = 0; d < D; ++d)
      elt[d] ^= p.elt[d];
    return *this;
  }

  // Binary operators
  point operator+(const point &p) const { return point(*this) += p; }
  point operator-(const point &p) const { return point(*this) -= p; }
  point operator*(const point &p) const { return point(*this) *= p; }
  point operator&(const point &p) const { return point(*this) &= p; }
  point operator|(const point &p) const { return point(*this) |= p; }
  point operator^(const point &p) const { return point(*this) ^= p; }
  point<bool, D> operator&&(const point &p) const {
    return point<bool, D>(*this) &= point<bool, D>(p);
  }
  point<bool, D> operator||(const point &p) const {
    return point<bool, D>(*this) |= point<bool, D>(p);
  }

  // Unary functions
  point abs() const {
    using std::abs;
    point r;
    for (int d = 0; d < D; ++d)
      r.elt[d] = abs(elt[d]);
    return r;
  }

  // Binary functions
  point min(const point &p) const {
    using std::min;
    point r;
    for (int d = 0; d < D; ++d)
      r.elt[d] = min(elt[d], p.elt[d]);
    return r;
  }
  point max(const point &p) const {
    using std::max;
    point r;
    for (int d = 0; d < D; ++d)
      r.elt[d] = max(elt[d], p.elt[d]);
    return r;
  }

  // Comparison operators
  point<bool, D> operator==(const point &p) const {
    point<bool, D> r;
    for (int d = 0; d < D; ++d)
      r.elt[d] = elt[d] == p.elt[d];
    return r;
  }
  point<bool, D> operator!=(const point &p) const { return !(*this == p); }
  point<bool, D> operator<(const point &p) const {
    point<bool, D> r;
    for (int d = 0; d < D; ++d)
      r.elt[d] = elt[d] < p.elt[d];
    return r;
  }
  point<bool, D> operator>(const point &p) const { return p < *this; }
  point<bool, D> operator>=(const point &p) const { return !(*this < p); }
  point<bool, D> operator<=(const point &p) const { return !(*this > p); }

  // Reductions
  bool all() const {
    bool r = true;
    for (int d = 0; d < D; ++d)
      r &= elt[d];
    return r;
  }
  bool any() const {
    bool r = false;
    for (int d = 0; d < D; ++d)
      r |= elt[d];
    return r;
  }
  T minval() const {
    using std::min;
    T r = std::numeric_limits<T>::max();
    for (int d = 0; d < D; ++d)
      r = min(r, elt[d]);
    return r;
  }
  T maxval() const {
    using std::max;
    T r = std::numeric_limits<T>::min();
    for (int d = 0; d < D; ++d)
      r = max(r, elt[d]);
    return r;
  }
  T sum() const {
    T r = T(0);
    for (int d = 0; d < D; ++d)
      r += elt[d];
    return r;
  }
  typedef typename detail::largeint<T>::type prod_t;
  prod_t prod() const {
    prod_t r = prod_t(1);
    for (int d = 0; d < D; ++d)
      r *= elt[d];
    return r;
  }

  // Output
  ostream &output(ostream &os) const {
    os << "[";
    for (int d = 0; d < D; ++d) {
      if (d > 0)
        os << ",";
      os << elt[d];
    }
    os << "]";
    return os;
  }
  friend ostream &operator<<(ostream &os, const point &p) {
    return p.output(os);
  }
};

// Unary functions
template <typename T, int D> point<T, D> abs(const point<T, D> &p) {
  return p.abs();
}

// Binary functions
template <typename T, int D>
point<T, D> min(const point<T, D> &p, const point<T, D> &q) {
  return p.min(q);
}
template <typename T, int D>
point<T, D> max(const point<T, D> &p, const point<T, D> &q) {
  return p.max(q);
}

// Reductions
template <typename T, int D> bool all(const point<T, D> &p) { return p.all(); }
template <typename T, int D> bool any(const point<T, D> &p) { return p.any(); }
template <typename T, int D> T minval(const point<T, D> &p) {
  return p.minval();
}
template <typename T, int D> T maxval(const point<T, D> &p) {
  return p.maxval();
}
template <typename T, int D> T sum(const point<T, D> &p) { return p.sum(); }
template <typename T, int D>
typename point<T, D>::prod_t prod(const point<T, D> &p) {
  return p.prod();
}

////////////////////////////////////////////////////////////////////////////////
// Box
////////////////////////////////////////////////////////////////////////////////

template <typename T, int D> struct box {
  point<T, D> lo, hi;
  box() = default;
  box(const box &b) = default;
  box(box &&b) = default;
  box(const point<T, D> &lo, const point<T, D> &hi) : lo(lo), hi(hi) {}
  box(const vector<T> &lo, const vector<T> &hi) : lo(lo), hi(hi) {}
  box &operator=(const box &p) = default;
  box &operator=(box &&p) = default;

  // Predicates
  bool empty() const { return any(hi <= lo); }
  point<T, D> lower() const { return empty() ? point<T, D>(0) : lo; }
  point<T, D> upper() const { return empty() ? point<T, D>(0) : hi; }
  point<T, D> shape() const { return max(hi - lo, point<T, D>(0)); }
  typedef typename point<T, D>::prod_t prod_t;
  prod_t size() const { return prod(shape()); }

  // Shift and scale operators
  box &operator>>=(const point<T, D> &p) {
    lo += p;
    hi += p;
    return *this;
  }
  box &operator<<=(const point<T, D> &p) {
    lo -= p;
    hi -= p;
    return *this;
  }
  box &operator*=(const point<T, D> &p) {
    lo *= p;
    hi *= p;
    return *this;
  }
  box operator>>(const point<T, D> &p) const { return box(*this) >>= p; }
  box operator<<(const point<T, D> &p) const { return box(*this) <<= p; }
  box operator*(const point<T, D> &p) const { return box(*this) *= p; }

  // Comparison operators
  bool operator==(const box &b) const {
    if (empty() && b.empty())
      return true;
    if (empty() || b.empty())
      return false;
    return all(lo == b.lo && hi == b.hi);
  }
  bool operator!=(const box &b) const { return !(*this == b); }

  // Set comparison operators
  bool contains(const point<T, D> &p) const {
    if (empty())
      return false;
    return all(p >= lo && p < hi);
  }
  bool isdisjoint(const box &b) const { return (*this & b).empty(); }
  bool operator<=(const box &b) const {
    if (empty())
      return true;
    if (b.empty())
      return false;
    return all(lo >= b.lo && hi <= b.hi);
  }
  bool operator>=(const box &b) const { return b <= *this; }
  bool operator<(const box &b) const { return *this <= b && *this != b; }
  bool operator>(const box &b) const { return b < *this; }
  bool issubset(const box &b) const { return *this <= b; }
  bool issuperset(const box &b) const { return *this >= b; }
  bool is_strict_subset(const box &b) const { return *this < b; }
  bool is_strict_superset(const box &b) const { return *this > b; }

  // Set operations
  box bounding_box(const box &b) const {
    if (empty())
      return b;
    if (b.empty())
      return *this;
    return box(min(lo, b.lo), max(hi, b.hi));
  }

  box operator&(const box &b) const {
    auto nlo = max(lo, b.lo);
    auto nhi = min(hi, b.hi);
    auto r = box(nlo, nhi);
// Postcondition
#if REGIONCALCULUS_DEBUG
    assert(r <= *this && r <= b);
#endif
    return r;
  }
  box intersection(const box &b) const { return *this & b; }

  void split(const point<T, D> &p, vector<box> &rs) const {
    assert(!empty());
#if REGIONCALCULUS_DEBUG
    const auto old_rs_size = rs.size();
#endif
    for (int m = 0; m < (1 << D); ++m) {
      point<T, D> newlo = lo, newhi = hi;
      bool is_inside = true;
      for (int d = 0; d < D; ++d) {
        const bool lohi = m & (1 << d);
        if (p.elt[d] > lo.elt[d] && p.elt[d] < hi.elt[d]) {
          if (!lohi)
            newhi.elt[d] = p.elt[d];
          else
            newlo.elt[d] = p.elt[d];
        } else {
          is_inside &= !lohi;
        }
      }
      if (is_inside)
        rs.push_back(box(newlo, newhi));
    }
#if REGIONCALCULUS_DEBUG
    // Postcondition
    prod_t vol = prod_t(0);
    for (auto i = old_rs_size; i < rs.size(); ++i) {
      assert(!rs[i].empty());
      assert(rs[i] <= *this);
      vol += rs[i].size();
    }
    assert(vol == size());
    for (std::size_t i = 0; i < rs.size(); ++i)
      for (std::size_t j = i + 1; j < rs.size(); ++j)
        assert(rs[i].isdisjoint(rs[j]));
#endif
  }

  vector<box> operator-(const box &b) const {
    if (empty())
      return vector<box>();
    if (b.empty())
      return vector<box>(1, *this);
    vector<box> bs1;
    split(b.lo, bs1);
    vector<box> bs2;
    for (const auto &b1 : bs1)
      b1.split(b.hi, bs2);
    vector<box> rs;
    for (const auto &b2 : bs2) {
      assert(b2.isdisjoint(b) || b2 <= b);
      if (b2.isdisjoint(b))
        rs.push_back(b2);
    }
#if REGIONCALCULUS_DEBUG
    // Postcondition
    prod_t vol = prod_t(0);
    for (const auto &r : rs) {
      assert(!r.empty());
      assert(r <= *this && !(r <= b));
      vol += r.size();
    }
    assert(vol >= max(prod_t(0), size() - b.size()) && vol <= size());
    for (std::size_t i = 0; i < rs.size(); ++i)
      for (std::size_t j = i + 1; j < rs.size(); ++j)
        assert(rs[i].isdisjoint(rs[j]));
#endif
    return rs;
  }
  vector<box> difference(const box &b) const { return *this - b; }

  vector<box> operator|(const box &b) const {
    auto rs = *this - b;
    if (!b.empty())
      rs.push_back(b);
#if REGIONCALCULUS_DEBUG
    // Postcondition
    prod_t vol = prod_t(0);
    for (const auto &r : rs) {
      assert(!r.empty());
      assert(r <= *this || r <= b);
      vol += r.size();
    }
    assert(vol >= size() && vol <= size() + b.size());
    for (std::size_t i = 0; i < rs.size(); ++i)
      for (std::size_t j = i + 1; j < rs.size(); ++j)
        assert(rs[i].isdisjoint(rs[j]));
#endif
    return rs;
  }
  vector<box> setunion(const box &b) const { return *this | b; }

  vector<box> operator^(const box &b) const {
    auto rs = *this - b;
    auto rs1 = b - *this;
    // TODO: Avoid this concatenation
    rs.insert(rs.end(), rs1.begin(), rs1.end());
#if REGIONCALCULUS_DEBUG
    // Postcondition
    prod_t vol = prod_t(0);
    for (const auto &r : rs) {
      assert(!r.empty());
      assert((r <= *this) ^ (r <= b));
      vol += r.size();
    }
    assert(vol >= abs(size() - b.size()) && vol <= size() + b.size());
    for (std::size_t i = 0; i < rs.size(); ++i)
      for (std::size_t j = i + 1; j < rs.size(); ++j)
        assert(rs[i].isdisjoint(rs[j]));
#endif
    return rs;
  }
  vector<box> symmetric_difference(const box &b) const { return *this ^ b; }

  // Output
  ostream &output(ostream &os) const {
    return os << "(" << lo << ":" << hi << ")";
  }
  friend ostream &operator<<(ostream &os, const box &b) { return b.output(os); }
};

////////////////////////////////////////////////////////////////////////////////
// Region
////////////////////////////////////////////////////////////////////////////////

template <typename T, int D> struct region {
  vector<box<T, D>> boxes;
  region() = default;
  region(const region &r) = default;
  region(region &&r) = default;
  region(const box<T, D> &b) {
    if (!b.empty())
      boxes.push_back(b);
  }
  region(const vector<box<T, D>> &bs) : boxes(bs) {}
  region(vector<box<T, D>> &&bs) : boxes(std::move(bs)) {}
  region &operator=(const region &r) = default;
  region &operator=(region &&r) = default;

  void append(const region &r) {
    boxes.insert(boxes.end(), r.boxes.begin(), r.boxes.end());
  }

  // Invariant
  bool invariant() const {
    for (std::size_t i = 0; i < boxes.size(); ++i) {
      if (boxes[i].empty())
        return false;
      for (std::size_t j = i + 1; j < boxes.size(); ++j) {
        if (!boxes[i].isdisjoint(boxes[j]))
          return false;
      }
    }
    return true;
  }

  // Predicates
  bool empty() const { return boxes.empty(); }
  typedef typename point<T, D>::prod_t prod_t;
  prod_t size() const {
    prod_t sz;
    for (const auto &b : boxes)
      sz += b.size();
    return sz;
  }

  // Set operations
  box<T, D> bounding_box() const {
    box<T, D> r;
    for (const auto &b : boxes)
      r = r.bounding_box(b);
    return r;
  }

  region operator&(const box<T, D> &b) const {
    region nr;
    for (const auto &rb : boxes) {
      auto nb = rb & b;
      if (!nb.empty())
        nr.boxes.push_back(nb);
    }
    return nr;
  }
  region operator&(const region &r) const {
    region nr;
    for (const auto &b : r.boxes)
      nr.append(*this & b);
    return nr;
  }
  region intersection(const box<T, D> &b) const { return *this & b; }
  region intersection(const region &r) const { return *this & r; }

  region operator-(const box<T, D> &b) const {
    region nr;
    for (const auto &rb : boxes)
      nr.append(region(rb - b));
    return nr;
  }
  region operator-(const region &r) const {
    region nr = *this;
    for (const auto &b : r.boxes)
      nr = nr - b;
    return nr;
  }
  region difference(const box<T, D> &b) const { return *this - b; }
  region difference(const region &r) const { return *this - r; }

  region operator|(const region &r) const {
    region nr = *this - r;
    nr.append(r);
    return nr;
  }
  region setunion(const region &r) const { return *this | r; }

  region operator^(const region &r) const {
    region nr = *this - r;
    nr.append(r - *this);
    return nr;
  }
  region symmetric_difference(const region &r) const { return *this ^ r; }

  // Set comparison operators
  bool contains(const point<T, D> &p) const {
    for (const auto &b : boxes)
      if (b.contains(p))
        return true;
    return false;
  }
  bool isdisjoint(const box<T, D> &b) const {
    for (const auto &rb : boxes)
      if (!rb.isdisjoint(b))
        return false;
    return true;
  }
  bool isdisjoint(const region &r) const {
    for (const auto &b : r.boxes)
      if (!isdisjoint(b))
        return false;
    return true;
  }

  // Comparison operators
  bool operator<=(const region &r) const { return (*this - r).empty(); }
  bool operator>=(const region &r) const { return r <= *this; }
  bool operator<(const region &r) const {
    return *this <= r && size() < r.size();
  }
  bool operator>(const region &r) const { return r < *this; }
  bool issubset(const region &r) const { return *this <= r; }
  bool issuperset(const region &r) const { return *this >= r; }
  bool is_strict_subset(const region &r) const { return *this < r; }
  bool is_strict_superset(const region &r) const { return *this > r; }
  bool operator==(const region &r) const { return (*this ^ r).empty(); }
  bool operator!=(const region &r) const { return !(*this == r); }

  // Output
  ostream &output(ostream &os) const {
    os << "{";
    for (std::size_t i = 0; i < boxes.size(); ++i) {
      if (i > 0)
        os << ",";
      os << boxes[i];
    }
    os << "}";
    return os;
  }
  friend ostream &operator<<(ostream &os, const region &r) {
    return r.output(os);
  }
};

////////////////////////////////////////////////////////////////////////////////
// Dimension-independent wrappers
////////////////////////////////////////////////////////////////////////////////

template <typename T, typename... Args>
unique_ptr<T> make_unique(Args &&... args) {
  return unique_ptr<T>(new T(std::forward<Args>(args)...));
}

// Virtual classes

template <typename T> struct vpoint {
  static unique_ptr<vpoint> make(int d);
  static unique_ptr<vpoint> make(const vector<T> &val);
  virtual unique_ptr<vpoint> copy() const = 0;

  virtual operator vector<T>() const = 0;

  virtual int rank() const = 0;

  // virtual unique_ptr<vpoint<T>> posit() const = 0;
  // virtual unique_ptr<vpoint<T>> negate() const = 0;

  virtual unique_ptr<vpoint<T>> plus(const unique_ptr<vpoint<T>> &p) const = 0;
  // virtual unique_ptr<vpoint<T>> minus(const vpoint<T> *p) const = 0;
  // virtual unique_ptr<vpoint<T>> multiplies(const vpoint<T> *p) const = 0;

  virtual ostream &output(ostream &os) const = 0;
  friend ostream &operator<<(ostream &os, const vpoint &p) {
    return p.output(os);
  }
};

template <typename T> struct vbox {
  virtual unique_ptr<vbox> copy() const = 0;

  static unique_ptr<vbox> make(int d);
  static unique_ptr<vbox> make(const unique_ptr<vpoint<T>> &lo,
                               const unique_ptr<vpoint<T>> &hi);

  virtual int rank() const = 0;
  virtual bool empty() const = 0;
  virtual unique_ptr<vpoint<T>> shape() const = 0;
  virtual unique_ptr<vpoint<T>> lower() const = 0;
  virtual unique_ptr<vpoint<T>> upper() const = 0;
  typedef typename box<T, 0>::prod_t prod_t;
  virtual prod_t size() const = 0;

  // virtual unique_ptr<vbox> bounding_box(const vbox *b) const = 0;

  virtual ostream &output(ostream &os) const = 0;
  friend ostream &operator<<(ostream &os, const vbox &b) {
    return b.output(os);
  }
};

template <typename T> struct vregion {
  static unique_ptr<vregion> make(int d);
  virtual unique_ptr<vregion> copy() const = 0;

  virtual int rank() const = 0;
  virtual bool empty() const = 0;
  typedef typename region<T, 0>::prod_t prod_t;
  virtual prod_t size() const = 0;

  virtual unique_ptr<vregion> bounding_box() const = 0;

  virtual ostream &output(ostream &os) const = 0;
  friend ostream &operator<<(ostream &os, const vregion &r) {
    return r.output(os);
  }
};

////////////////////////////////////////////////////////////////////////////////

// Wrapper classes (using pointers)

template <typename T, int D> struct wpoint : vpoint<T> {
  point<T, D> val;

  unique_ptr<vpoint<T>> copy() const { return make_unique<wpoint>(*this); }

  wpoint(const vector<T> &p) : val(p) {}

  operator vector<T>() const { return vector<T>(val); }

  int rank() const { return D; }

  unique_ptr<vpoint<T>> plus(const unique_ptr<vpoint<T>> &p) const {
    return make_unique<wpoint>(val +
                               dynamic_cast<const wpoint *>(p.get())->val);
  }

  ostream &output(ostream &os) const { return val.output(os); }
  friend ostream &operator<<(ostream &os, const wpoint &p) {
    return p.output(os);
  }
};

template <typename T> unique_ptr<vpoint<T>> vpoint<T>::make(int d) {
  switch (d) {
  case 0:
    return make_unique<wpoint<T, 0>>();
  case 1:
    return make_unique<wpoint<T, 1>>();
  case 2:
    return make_unique<wpoint<T, 2>>();
  case 3:
    return make_unique<wpoint<T, 3>>();
  case 4:
    return make_unique<wpoint<T, 4>>();
  default:
    assert(0);
  }
}

template <typename T>
unique_ptr<vpoint<T>> vpoint<T>::make(const vector<T> &val) {
  switch (val.size()) {
  case 0:
    return make_unique<wpoint<T, 0>>(val);
  case 1:
    return make_unique<wpoint<T, 1>>(val);
  case 2:
    return make_unique<wpoint<T, 2>>(val);
  case 3:
    return make_unique<wpoint<T, 3>>(val);
  case 4:
    return make_unique<wpoint<T, 4>>(val);
  default:
    assert(0);
  }
}

template <typename T, int D> struct wbox : vbox<T> {
  box<T, D> val;

  wbox(const wpoint<T, D> *lo, const wpoint<T, D> *hi)
      : val(lo->val, hi->val) {}

  unique_ptr<vbox<T>> copy() const { return make_unique<wbox>(*this); }

  int rank() const { return D; }
  bool empty() const { return val.empty(); }
  unique_ptr<vpoint<T>> lower() const {
    return make_unique<wpoint<T, D>>(val.lower());
  }
  unique_ptr<vpoint<T>> upper() const {
    return make_unique<wpoint<T, D>>(val.upper());
  }
  unique_ptr<vpoint<T>> shape() const {
    return make_unique<wpoint<T, D>>(val.shape());
  }
  typedef typename vbox<T>::prod_t prod_t;
  prod_t size() const { return val.size(); }

  // unique_ptr<wbox> bounding_box(const wbox *b) const {
  //   return val.bounding_box(*b);
  // }

  ostream &output(ostream &os) const { return val.output(os); }
  friend ostream &operator<<(ostream &os, const wbox &b) {
    return b.output(os);
  }
};

template <typename T> unique_ptr<vbox<T>> vbox<T>::make(int d) {
  switch (d) {
  case 0:
    return make_unique<wbox<T, 0>>();
  case 1:
    return make_unique<wbox<T, 1>>();
  case 2:
    return make_unique<wbox<T, 2>>();
  case 3:
    return make_unique<wbox<T, 3>>();
  case 4:
    return make_unique<wbox<T, 4>>();
  default:
    assert(0);
  }
}

template <typename T>
unique_ptr<vbox<T>> vbox<T>::make(const unique_ptr<vpoint<T>> &lo,
                                  const unique_ptr<vpoint<T>> &hi) {
  switch (lo->rank()) {
  case 0:
    return make_unique<wbox<T, 0>>(
        dynamic_cast<const wpoint<T, 0> *>(lo.get()),
        dynamic_cast<const wpoint<T, 0> *>(hi.get()));
  case 1:
    return make_unique<wbox<T, 1>>(
        dynamic_cast<const wpoint<T, 1> *>(lo.get()),
        dynamic_cast<const wpoint<T, 1> *>(hi.get()));
  case 2:
    return make_unique<wbox<T, 2>>(
        dynamic_cast<const wpoint<T, 2> *>(lo.get()),
        dynamic_cast<const wpoint<T, 2> *>(hi.get()));
  case 3:
    return make_unique<wbox<T, 3>>(
        dynamic_cast<const wpoint<T, 3> *>(lo.get()),
        dynamic_cast<const wpoint<T, 3> *>(hi.get()));
  case 4:
    return make_unique<wbox<T, 4>>(
        dynamic_cast<const wpoint<T, 4> *>(lo.get()),
        dynamic_cast<const wpoint<T, 4> *>(hi.get()));
  default:
    assert(0);
  }
}

template <typename T, int D> struct wregion : vregion<T> {
  region<T, D> val;

  unique_ptr<vregion<T>> copy() const {
    return unique_ptr<vregion<T>>(new wregion(*this));
  }

  int rank() const { return D; }
  bool empty() const { return val.empty(); }
  typedef typename vregion<T>::prod_t prod_t;
  prod_t size() const { return val.size(); }

  unique_ptr<wbox<T, D>> bounding_box() const { return val.bounding_box(); }

  ostream &output(ostream &os) const { return val.output(os); }
  friend ostream &operator<<(ostream &os, const wregion &r) {
    return r.output(os);
  }
};

template <typename T> unique_ptr<vregion<T>> vregion<T>::make(int d) {
  switch (d) {
  case 0:
    return new wregion<T, 0>();
  case 1:
    return new wregion<T, 1>();
  case 2:
    return new wregion<T, 2>();
  case 3:
    return new wregion<T, 3>();
  case 4:
    return new wregion<T, 4>();
  default:
    assert(0);
  }
}

////////////////////////////////////////////////////////////////////////////////

// Dimension-independent classes (hiding the pointers)

template <typename T> struct dpoint {
  unique_ptr<vpoint<T>> val;

  dpoint() = default;

  dpoint(int d) : val(vpoint<T>::make(d)) {}
  dpoint(const vpoint<T> *val) : val(vpoint<T>::make(val)) {}
  dpoint(unique_ptr<vpoint<T>> &&val) : val(std::move(val)) {}

  dpoint(const dpoint &p) : val(p.val.copy()) {}
  dpoint(dpoint &&p) = default;
  dpoint &operator=(const dpoint &p) {
    val = p.val->copy();
    return *this;
  }
  dpoint &operator=(dpoint &&p) = default;

  dpoint(const vector<T> &p) : val(vpoint<T>::make(p)) {}
  operator vector<T>() const { return vector<T>(*val); }

  operator bool() const { return bool(val); }
  void reset() { val.reset(); }

  int rank() const { return val->rank(); }

  dpoint operator+() const { return dpoint(val->posit()); }
  dpoint operator-() const { return dpoint(val->negate()); }

  dpoint operator+(const dpoint &p) const { return dpoint(val->plus(p.val)); }
  dpoint operator-(const dpoint &p) const { return dpoint(val->minus(p.val)); }
  dpoint operator*(const dpoint &p) const {
    return dpoint(val->multiplies(p.val));
  }

  ostream &output(ostream &os) const { return val->output(os); }
  friend ostream &operator<<(ostream &os, const dpoint &p) {
    return p.output(os);
  }
};

template <typename T> struct dbox {
  unique_ptr<vbox<T>> val;

  dbox() = default;

  dbox(int d) : val(vbox<T>::make(d)) {}
  dbox(const vbox<T> *val) : val(vbox<T>::make(val)) {}
  dbox(unique_ptr<vbox<T>> &&val) : val(std::move(val)) {}

  dbox(const dbox &b) : val(b.val.copy()) {}
  dbox(dbox &&b) = default;
  dbox &operator=(const dbox &b) {
    val = b.val->copy();
    return *this;
  }
  dbox &operator=(dbox &&b) = default;

  dbox(const dpoint<T> &lo, const dpoint<T> &hi)
      : val(vbox<T>::make(lo.val, hi.val)) {}

  operator bool() const { return bool(val); }
  void reset() { val.reset(); }

  int rank() const { return val->rank(); }
  bool empty() const { return val->empty(); }
  dpoint<T> lower() const { return dpoint<T>(val->lower()); }
  dpoint<T> upper() const { return dpoint<T>(val->upper()); }
  dpoint<T> shape() const { return dpoint<T>(val->shape()); }
  typedef typename vbox<T>::prod_t prod_t;
  prod_t size() const { return val->size(); }

  dbox bounding_box(const dbox &b) const {
    return dbox(val->bounding_box(b->val));
  }

  ostream &output(ostream &os) const { return val->output(os); }
  friend ostream &operator<<(ostream &os, const dbox &b) {
    return b.output(os);
  }
};

template <typename T> struct dregion {
  unique_ptr<vregion<T>> val;

  dregion() = default;

  dregion(int d) : val(vregion<T>::make(d)) {}
  dregion(const vregion<T> *val) : val(vregion<T>::make(val)) {}
  dregion(unique_ptr<vregion<T>> &&val) : val(std::move(val)) {}

  dregion(const dregion &r) : val(r.val.copy()) {}
  dregion(dregion &&r) = default;
  dregion &operator=(const dregion &r) {
    val = r.val->copy();
    return *this;
  }
  dregion &operator=(dregion &&r) = default;

  operator bool() const { return bool(val); }
  void reset() { val.reset(); }

  int rank() const { return val->rank(); }
  bool empty() const { return val->empty(); }
  typedef typename vregion<T>::prod_t prod_t;
  prod_t size() const { return val->size(); }

  dregion bounding_box() const { return dregion(val->bounding_box()); }

  ostream &output(ostream &os) const { return val->output(os); }
  friend ostream &operator<<(ostream &os, const dregion &r) {
    return r.output(os);
  }
};
}

#endif // REGIONCALCULUS_HPP
