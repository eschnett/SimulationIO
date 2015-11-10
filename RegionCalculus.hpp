#ifndef REGIONCALCULUS_HPP
#define REGIONCALCULUS_HPP

#include <algorithm>
#include <array>
#include <cassert>
#include <functional>
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
template <> struct largeint<unsigned short> {
  typedef unsigned long long type;
};
template <> struct largeint<unsigned int> { typedef unsigned long long type; };
template <> struct largeint<unsigned long> { typedef unsigned long long type; };
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
  point &operator/=(const point &p) {
    for (int d = 0; d < D; ++d)
      elt[d] /= p.elt[d];
    return *this;
  }
  point &operator%=(const point &p) {
    for (int d = 0; d < D; ++d)
      elt[d] %= p.elt[d];
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
  point operator/(const point &p) const { return point(*this) /= p; }
  point operator%(const point &p) const { return point(*this) %= p; }
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

  bool equal_to(const point &p) const { return elt == p.elt; }
  bool less(const point &p) const { return elt < p.elt; }

  // Reductions
  bool all() const {
    bool r = true;
    for (int d = 0; d < D; ++d)
      r = r && elt[d];
    return r;
  }
  bool any() const {
    bool r = false;
    for (int d = 0; d < D; ++d)
      r = r || elt[d];
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
}

namespace std {
template <typename T, int D> struct equal_to<RegionCalculus::point<T, D>> {
  bool operator()(const RegionCalculus::point<T, D> &p,
                  const RegionCalculus::point<T, D> &q) const {
    return p.equal_to(q);
  }
};
template <typename T, int D> struct less<RegionCalculus::point<T, D>> {
  bool operator()(const RegionCalculus::point<T, D> &p,
                  const RegionCalculus::point<T, D> &q) const {
    return p.less(q);
  }
};
}

////////////////////////////////////////////////////////////////////////////////
// Box
////////////////////////////////////////////////////////////////////////////////

namespace RegionCalculus {
template <typename T, int D> struct box {
  point<T, D> lo, hi;
  box() = default;
  box(const box &b) = default;
  box(box &&b) = default;
  box(const point<T, D> &lo, const point<T, D> &hi) : lo(lo), hi(hi) {}
  box(const vector<T> &lo, const vector<T> &hi) : lo(lo), hi(hi) {}
  box &operator=(const box &p) = default;
  box &operator=(box &&p) = default;
  template <typename U> box(const box<U, D> &b) : lo(b.lo), hi(b.hi) {}

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
  bool less(const box &b) const {
    if (b.empty())
      return false;
    if (empty())
      return true;
    std::less<point<T, D>> cmp_less;
    if (cmp_less(lo, b.lo))
      return true;
    if (cmp_less(b.lo, lo))
      return false;
    return cmp_less(hi, b.hi);
  }

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

private:
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

public:
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
}

namespace std {
template <typename T, int D> struct less<RegionCalculus::box<T, D>> {
  bool operator()(const RegionCalculus::box<T, D> &p,
                  const RegionCalculus::box<T, D> &q) const {
    return p.less(q);
  }
};
}

////////////////////////////////////////////////////////////////////////////////
// Region
////////////////////////////////////////////////////////////////////////////////

namespace RegionCalculus {
template <typename T, int D> struct region {
  vector<box<T, D>> boxes;
  region() = default;
  region(const region &r) = default;
  region(region &&r) = default;

  region(const box<T, D> &b) {
    if (!b.empty())
      boxes.push_back(b);
#if REGIONCALCULUS_DEBUG
    assert(invariant());
#endif
  }
  region(const vector<box<T, D>> &bs) : boxes(bs) {
    normalize();
    assert(invariant());
  }
  region(vector<box<T, D>> &&bs) : boxes(std::move(bs)) {
    normalize();
    assert(invariant());
  }
  operator vector<box<T, D>>() const { return boxes; }
  region &operator=(const region &r) = default;
  region &operator=(region &&r) = default;
  template <typename U> region(const region<U, D> &r) {
    boxes.reserve(r.boxes.size());
    for (const auto &b : r.boxes)
      boxes.push_back(box<T, D>(b));
    assert(invariant());
  }

private:
  void append(const region &r) {
    boxes.insert(boxes.end(), r.boxes.begin(), r.boxes.end());
  }

  // Normalization
  void normalize() {
    std::sort(boxes.begin(), boxes.end(), std::less<box<T, D>>());
  }

public:
  // Invariant
  bool invariant() const {
    for (std::size_t i = 0; i < boxes.size(); ++i) {
      if (boxes[i].empty())
        return false;
      for (std::size_t j = i + 1; j < boxes.size(); ++j) {
        if (!boxes[i].isdisjoint(boxes[j]))
          return false;
        if (!boxes[i].less(boxes[j]))
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
    nr.normalize();
#if REGIONCALCULUS_DEBUG
    assert(invariant());
#endif
    return nr;
  }
  region operator&(const region &r) const {
    region nr;
    for (const auto &b : r.boxes)
      nr.append(*this & b);
    nr.normalize();
#if REGIONCALCULUS_DEBUG
    assert(invariant());
#endif
    return nr;
  }
  region intersection(const box<T, D> &b) const { return *this & b; }
  region intersection(const region &r) const { return *this & r; }

  region operator-(const box<T, D> &b) const {
    region nr;
    for (const auto &rb : boxes)
      nr.append(region(rb - b));
    nr.normalize();
#if REGIONCALCULUS_DEBUG
    assert(invariant());
#endif
    return nr;
  }
  region operator-(const region &r) const {
    region nr = *this;
    for (const auto &b : r.boxes)
      nr = nr - b;
    nr.normalize();
#if REGIONCALCULUS_DEBUG
    assert(invariant());
#endif
    return nr;
  }
  region difference(const box<T, D> &b) const { return *this - b; }
  region difference(const region &r) const { return *this - r; }

  region operator|(const region &r) const {
    region nr = *this - r;
    nr.append(r);
    nr.normalize();
#if REGIONCALCULUS_DEBUG
    assert(invariant());
#endif
    return nr;
  }
  region setunion(const region &r) const { return *this | r; }

  region operator^(const region &r) const {
    region nr = *this - r;
    nr.append(r - *this);
    nr.normalize();
#if REGIONCALCULUS_DEBUG
    assert(invariant());
#endif
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

  bool less(const region &r) const { return boxes < r.boxes; }

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
}

namespace std {
template <typename T, int D> struct less<RegionCalculus::region<T, D>> {
  bool operator()(const RegionCalculus::region<T, D> &p,
                  const RegionCalculus::region<T, D> &q) const {
    return p.less(q);
  }
};
}

////////////////////////////////////////////////////////////////////////////////
// Dimension-independent wrappers
////////////////////////////////////////////////////////////////////////////////

namespace RegionCalculus {
template <typename T, typename... Args>
unique_ptr<T> make_unique(Args &&... args) {
  return unique_ptr<T>(new T(std::forward<Args>(args)...));
}

// Virtual classes

template <typename T> struct vpoint {
  virtual unique_ptr<vpoint> copy() const = 0;

  static unique_ptr<vpoint> make(int d);
  static unique_ptr<vpoint> make(int d, const T &x);
  static unique_ptr<vpoint> make(const vector<T> &val);
  virtual operator vector<T>() const = 0;
  template <typename U> static unique_ptr<vpoint> make(const vpoint<U> &p);

  virtual int rank() const = 0;

  // Unary operators
  virtual unique_ptr<vpoint> operator+() const = 0;
  virtual unique_ptr<vpoint> operator-() const = 0;
  virtual unique_ptr<vpoint> operator~() const = 0;
  virtual unique_ptr<vpoint<bool>> operator!() const = 0;

  // Assignment operators
  virtual vpoint &operator+=(const vpoint &p) = 0;
  virtual vpoint &operator-=(const vpoint &p) = 0;
  virtual vpoint &operator*=(const vpoint &p) = 0;
  virtual vpoint &operator/=(const vpoint &p) = 0;
  virtual vpoint &operator%=(const vpoint &p) = 0;
  virtual vpoint &operator&=(const vpoint &p) = 0;
  virtual vpoint &operator|=(const vpoint &p) = 0;
  virtual vpoint &operator^=(const vpoint &p) = 0;

  // Binary operators
  virtual unique_ptr<vpoint> operator+(const vpoint &p) const = 0;
  virtual unique_ptr<vpoint> operator-(const vpoint &p) const = 0;
  virtual unique_ptr<vpoint> operator*(const vpoint &p) const = 0;
  virtual unique_ptr<vpoint> operator/(const vpoint &p) const = 0;
  virtual unique_ptr<vpoint> operator%(const vpoint &p) const = 0;
  virtual unique_ptr<vpoint> operator&(const vpoint &p) const = 0;
  virtual unique_ptr<vpoint> operator|(const vpoint &p) const = 0;
  virtual unique_ptr<vpoint> operator^(const vpoint &p) const = 0;
  virtual unique_ptr<vpoint<bool>> operator&&(const vpoint &p) const = 0;
  virtual unique_ptr<vpoint<bool>> operator||(const vpoint &p) const = 0;

  // Unary functions
  virtual unique_ptr<vpoint> abs() const = 0;

  // Binary functions
  virtual unique_ptr<vpoint> min(const vpoint &p) const = 0;
  virtual unique_ptr<vpoint> max(const vpoint &p) const = 0;

  // Comparison operators
  virtual unique_ptr<vpoint<bool>> operator==(const vpoint &p) const = 0;
  virtual unique_ptr<vpoint<bool>> operator!=(const vpoint &p) const = 0;
  virtual unique_ptr<vpoint<bool>> operator<(const vpoint &p) const = 0;
  virtual unique_ptr<vpoint<bool>> operator>(const vpoint &p) const = 0;
  virtual unique_ptr<vpoint<bool>> operator<=(const vpoint &p) const = 0;
  virtual unique_ptr<vpoint<bool>> operator>=(const vpoint &p) const = 0;

  virtual bool equal_to(const vpoint &p) const = 0;
  virtual bool less(const vpoint &p) const = 0;

  // Reductions
  virtual bool any() const = 0;
  virtual bool all() const = 0;
  virtual T minval() const = 0;
  virtual T maxval() const = 0;
  virtual T sum() const = 0;
  typedef typename point<T, 0>::prod_t prod_t;
  virtual prod_t prod() const = 0;

  // Output
  virtual ostream &output(ostream &os) const = 0;
  friend ostream &operator<<(ostream &os, const vpoint &p) {
    return p.output(os);
  }
};

template <typename T> struct vbox {
  virtual unique_ptr<vbox> copy() const = 0;

  static unique_ptr<vbox> make(int d);
  static unique_ptr<vbox> make(const vpoint<T> &lo, const vpoint<T> &hi);
  template <typename U> static unique_ptr<vbox> make(const vbox<U> &p);

  virtual int rank() const = 0;

  // Predicates
  virtual bool empty() const = 0;
  virtual unique_ptr<vpoint<T>> shape() const = 0;
  virtual unique_ptr<vpoint<T>> lower() const = 0;
  virtual unique_ptr<vpoint<T>> upper() const = 0;
  typedef typename box<T, 0>::prod_t prod_t;
  virtual prod_t size() const = 0;

  // Shift and scale operators
  virtual vbox &operator>>=(const vpoint<T> &p) = 0;
  virtual vbox &operator<<=(const vpoint<T> &p) = 0;
  virtual vbox &operator*=(const vpoint<T> &p) = 0;
  virtual unique_ptr<vbox> operator>>(const vpoint<T> &p) const = 0;
  virtual unique_ptr<vbox> operator<<(const vpoint<T> &p) const = 0;
  virtual unique_ptr<vbox> operator*(const vpoint<T> &p) const = 0;

  // Comparison operators
  virtual bool operator==(const vbox &b) const = 0;
  virtual bool operator!=(const vbox &b) const = 0;

  virtual bool less(const vbox &b) const = 0;

  // Set comparison operators
  virtual bool contains(const vpoint<T> &p) const = 0;
  virtual bool isdisjoint(const vbox &b) const = 0;
  virtual bool operator<=(const vbox &b) const = 0;
  virtual bool operator>=(const vbox &b) const = 0;
  virtual bool operator<(const vbox &b) const = 0;
  virtual bool operator>(const vbox &b) const = 0;

  // Set operations
  virtual unique_ptr<vbox> bounding_box(const vbox &b) const = 0;
  virtual unique_ptr<vbox> operator&(const vbox &b) const = 0;
  // virtual unique_ptr<vbox> operator-(const vbox &b) const = 0;
  // virtual unique_ptr<vbox> operator|(const vbox &b) const = 0;
  // virtual unique_ptr<vbox> operator^(const vbox &b) const = 0;

  // Output
  virtual ostream &output(ostream &os) const = 0;
  friend ostream &operator<<(ostream &os, const vbox &b) {
    return b.output(os);
  }
};

template <typename T> struct vregion {
  virtual unique_ptr<vregion> copy() const = 0;

  static unique_ptr<vregion> make(int d);
  static unique_ptr<vregion> make(const vbox<T> &b);
  static unique_ptr<vregion> make(const vector<unique_ptr<vbox<T>>> &bs);
  virtual operator vector<unique_ptr<vbox<T>>>() const = 0;
  template <typename U> static unique_ptr<vregion> make(const vregion<U> &p);

  virtual int rank() const = 0;

  // Predicates
  virtual bool invariant() const = 0;
  virtual bool empty() const = 0;
  typedef typename region<T, 0>::prod_t prod_t;
  virtual prod_t size() const = 0;

  // Set operations
  virtual unique_ptr<vbox<T>> bounding_box() const = 0;
  virtual unique_ptr<vregion> operator&(const vbox<T> &b) const = 0;
  virtual unique_ptr<vregion> operator&(const vregion &r) const = 0;
  virtual unique_ptr<vregion> operator-(const vbox<T> &b) const = 0;
  virtual unique_ptr<vregion> operator-(const vregion &r) const = 0;
  virtual unique_ptr<vregion> operator|(const vbox<T> &b) const = 0;
  virtual unique_ptr<vregion> operator|(const vregion &r) const = 0;
  virtual unique_ptr<vregion> operator^(const vbox<T> &b) const = 0;
  virtual unique_ptr<vregion> operator^(const vregion &r) const = 0;

  // Set comparison operators
  virtual bool contains(const vpoint<T> &p) const = 0;
  virtual bool isdisjoint(const vbox<T> &b) const = 0;
  virtual bool isdisjoint(const vregion &r) const = 0;

  // Comparison operators
  virtual bool operator<=(const vregion &r) const = 0;
  virtual bool operator>=(const vregion &r) const = 0;
  virtual bool operator<(const vregion &r) const = 0;
  virtual bool operator>(const vregion &r) const = 0;
  virtual bool operator==(const vregion &r) const = 0;
  virtual bool operator!=(const vregion &r) const = 0;

  virtual bool less(const vregion &r) const = 0;

  // Output
  virtual ostream &output(ostream &os) const = 0;
  friend ostream &operator<<(ostream &os, const vregion &r) {
    return r.output(os);
  }
};

////////////////////////////////////////////////////////////////////////////////

// Wrapper classes (using pointers)

template <typename T, int D> struct wpoint : vpoint<T> {
  point<T, D> val;

  wpoint(const wpoint &p) = default;
  wpoint(wpoint &&p) = default;
  wpoint &operator=(const wpoint &p) = default;
  wpoint &operator=(wpoint &&p) = default;
  unique_ptr<vpoint<T>> copy() const { return make_unique<wpoint>(*this); }

  wpoint(const point<T, D> &p) : val(p) {}

  wpoint() : val() {}
  wpoint(const T &x) : val(x) {}
  wpoint(const array<T, D> &p) : val(p) {}
  wpoint(const vector<T> &p) : val(p) {}
  operator vector<T>() const { return vector<T>(val); }
  template <typename U> wpoint(const wpoint<U, D> &p) : val(p.val) {}

  int rank() const { return D; }

  // Unary operators
  unique_ptr<vpoint<T>> operator+() const { return make_unique<wpoint>(+val); }
  unique_ptr<vpoint<T>> operator-() const { return make_unique<wpoint>(-val); }
  unique_ptr<vpoint<T>> operator~() const { return make_unique<wpoint>(~val); }
  unique_ptr<vpoint<bool>> operator!() const {
    return make_unique<wpoint<bool, D>>(!val);
  }

  // Assignment operators
  vpoint<T> &operator+=(const vpoint<T> &p) {
    val += dynamic_cast<const wpoint &>(p).val;
    return *this;
  }
  vpoint<T> &operator-=(const vpoint<T> &p) {
    val -= dynamic_cast<const wpoint &>(p).val;
    return *this;
  }
  vpoint<T> &operator*=(const vpoint<T> &p) {
    val *= dynamic_cast<const wpoint &>(p).val;
    return *this;
  }
  vpoint<T> &operator/=(const vpoint<T> &p) {
    val /= dynamic_cast<const wpoint &>(p).val;
    return *this;
  }
  vpoint<T> &operator%=(const vpoint<T> &p) {
    val %= dynamic_cast<const wpoint &>(p).val;
    return *this;
  }
  vpoint<T> &operator&=(const vpoint<T> &p) {
    val &= dynamic_cast<const wpoint &>(p).val;
    return *this;
  }
  vpoint<T> &operator|=(const vpoint<T> &p) {
    val |= dynamic_cast<const wpoint &>(p).val;
    return *this;
  }
  vpoint<T> &operator^=(const vpoint<T> &p) {
    val ^= dynamic_cast<const wpoint &>(p).val;
    return *this;
  }

  // Binary operators
  unique_ptr<vpoint<T>> operator+(const vpoint<T> &p) const {
    return make_unique<wpoint>(val + dynamic_cast<const wpoint &>(p).val);
  }
  unique_ptr<vpoint<T>> operator-(const vpoint<T> &p) const {
    return make_unique<wpoint>(val - dynamic_cast<const wpoint &>(p).val);
  }
  unique_ptr<vpoint<T>> operator*(const vpoint<T> &p) const {
    return make_unique<wpoint>(val * dynamic_cast<const wpoint &>(p).val);
  }
  unique_ptr<vpoint<T>> operator/(const vpoint<T> &p) const {
    return make_unique<wpoint>(val / dynamic_cast<const wpoint &>(p).val);
  }
  unique_ptr<vpoint<T>> operator%(const vpoint<T> &p) const {
    return make_unique<wpoint>(val % dynamic_cast<const wpoint &>(p).val);
  }
  unique_ptr<vpoint<T>> operator&(const vpoint<T> &p) const {
    return make_unique<wpoint>(val & dynamic_cast<const wpoint &>(p).val);
  }
  unique_ptr<vpoint<T>> operator|(const vpoint<T> &p) const {
    return make_unique<wpoint>(val | dynamic_cast<const wpoint &>(p).val);
  }
  unique_ptr<vpoint<T>> operator^(const vpoint<T> &p) const {
    return make_unique<wpoint>(val ^ dynamic_cast<const wpoint &>(p).val);
  }
  unique_ptr<vpoint<bool>> operator&&(const vpoint<T> &p) const {
    return make_unique<wpoint<bool, D>>(val &&
                                        dynamic_cast<const wpoint &>(p).val);
  }
  unique_ptr<vpoint<bool>> operator||(const vpoint<T> &p) const {
    return make_unique<wpoint<bool, D>>(val ||
                                        dynamic_cast<const wpoint &>(p).val);
  }

  // Unary functions
  unique_ptr<vpoint<T>> abs() const { return make_unique<wpoint>(val.abs()); }

  // Binary functions
  unique_ptr<vpoint<T>> min(const vpoint<T> &p) const {
    return make_unique<wpoint>(val.min(dynamic_cast<const wpoint &>(p).val));
  }
  unique_ptr<vpoint<T>> max(const vpoint<T> &p) const {
    return make_unique<wpoint>(val.max(dynamic_cast<const wpoint &>(p).val));
  }

  // Comparison operators
  unique_ptr<vpoint<bool>> operator==(const vpoint<T> &p) const {
    return make_unique<wpoint<bool, D>>(val ==
                                        dynamic_cast<const wpoint &>(p).val);
  }
  unique_ptr<vpoint<bool>> operator!=(const vpoint<T> &p) const {
    return make_unique<wpoint<bool, D>>(val !=
                                        dynamic_cast<const wpoint &>(p).val);
  }
  unique_ptr<vpoint<bool>> operator<(const vpoint<T> &p) const {
    return make_unique<wpoint<bool, D>>(val <
                                        dynamic_cast<const wpoint &>(p).val);
  }
  unique_ptr<vpoint<bool>> operator>(const vpoint<T> &p) const {
    return make_unique<wpoint<bool, D>>(val >
                                        dynamic_cast<const wpoint &>(p).val);
  }
  unique_ptr<vpoint<bool>> operator<=(const vpoint<T> &p) const {
    return make_unique<wpoint<bool, D>>(val <=
                                        dynamic_cast<const wpoint &>(p).val);
  }
  unique_ptr<vpoint<bool>> operator>=(const vpoint<T> &p) const {
    return make_unique<wpoint<bool, D>>(val >=
                                        dynamic_cast<const wpoint &>(p).val);
  }

  bool equal_to(const vpoint<T> &p) const {
    return val.equal_to(dynamic_cast<const wpoint &>(p).val);
  }
  bool less(const vpoint<T> &p) const {
    return val.less(dynamic_cast<const wpoint &>(p).val);
  }

  // Reductions
  bool any() const { return val.any(); }
  bool all() const { return val.all(); }
  T minval() const { return val.minval(); }
  T maxval() const { return val.maxval(); }
  T sum() const { return val.sum(); }
  typedef typename vpoint<T>::prod_t prod_t;
  prod_t prod() const { return val.prod(); }

  // Output
  ostream &output(ostream &os) const { return val.output(os); }
  friend ostream &operator<<(ostream &os, const wpoint &p) {
    return p.output(os);
  }
};

template <typename T, int D> struct wbox : vbox<T> {
  box<T, D> val;

  wbox(const wbox &b) = default;
  wbox(wbox &&b) = default;
  wbox &operator=(const wbox &b) = default;
  wbox &operator=(wbox &&b) = default;
  unique_ptr<vbox<T>> copy() const { return make_unique<wbox>(*this); }

  wbox(const box<T, D> &b) : val(b) {}

  wbox() : val() {}
  wbox(const wpoint<T, D> &lo, const wpoint<T, D> &hi) : val(lo.val, hi.val) {}
  template <typename U> wbox(const wbox<U, D> &p) : val(p.val) {}

  int rank() const { return D; }

  // Predicates
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

  // Shift and scale operators
  vbox<T> &operator>>=(const vpoint<T> &p) {
    val >>= dynamic_cast<const wpoint<T, D> &>(p).val;
    return *this;
  }
  vbox<T> &operator<<=(const vpoint<T> &p) {
    val <<= dynamic_cast<const wpoint<T, D> &>(p).val;
    return *this;
  }
  vbox<T> &operator*=(const vpoint<T> &p) {
    val *= dynamic_cast<const wpoint<T, D> &>(p).val;
    return *this;
  }
  unique_ptr<vbox<T>> operator>>(const vpoint<T> &p) const {
    return make_unique<wbox>(val >> dynamic_cast<const wpoint<T, D> &>(p).val);
  }
  unique_ptr<vbox<T>> operator<<(const vpoint<T> &p) const {
    return make_unique<wbox>(val << dynamic_cast<const wpoint<T, D> &>(p).val);
  }
  unique_ptr<vbox<T>> operator*(const vpoint<T> &p) const {
    return make_unique<wbox>(val * dynamic_cast<const wpoint<T, D> &>(p).val);
  }

  // Comparison operators
  bool operator==(const vbox<T> &b) const {
    return val == dynamic_cast<const wbox<T, D> &>(b).val;
  }
  bool operator!=(const vbox<T> &b) const {
    return val != dynamic_cast<const wbox<T, D> &>(b).val;
  }

  bool less(const vbox<T> &b) const {
    return val.less(dynamic_cast<const wbox<T, D> &>(b).val);
  }

  // Set comparison operators
  bool contains(const vpoint<T> &p) const {
    return val.contains(dynamic_cast<const wpoint<T, D> &>(p).val);
  }
  bool isdisjoint(const vbox<T> &b) const {
    return val.isdisjoint(dynamic_cast<const wbox &>(b).val);
  }
  bool operator<=(const vbox<T> &b) const {
    return val <= dynamic_cast<const wbox &>(b).val;
  }
  bool operator>=(const vbox<T> &b) const {
    return val >= dynamic_cast<const wbox &>(b).val;
  }
  bool operator<(const vbox<T> &b) const {
    return val < dynamic_cast<const wbox &>(b).val;
  }
  bool operator>(const vbox<T> &b) const {
    return val > dynamic_cast<const wbox &>(b).val;
  }

  // Set operations
  unique_ptr<vbox<T>> bounding_box(const vbox<T> &b) const {
    return make_unique<wbox>(
        val.bounding_box(dynamic_cast<const wbox &>(b).val));
  }
  unique_ptr<vbox<T>> operator&(const vbox<T> &b) const {
    return make_unique<wbox>(val & dynamic_cast<const wbox &>(b).val);
  }
  // unique_ptr<vbox<T>> operator-(const vbox<T> &b) const {
  //   return make_unique<wbox>(val - dynamic_cast<const wbox &>(b).val);
  // }
  // unique_ptr<vbox<T>> operator|(const vbox<T> &b) const {
  //   return make_unique<wbox>(val | dynamic_cast<const wbox &>(b).val);
  // }
  // unique_ptr<vbox<T>> operator^(const vbox<T> &b) const {
  //   return make_unique<wbox>(val ^ dynamic_cast<const wbox &>(b).val);
  // }

  // Output
  ostream &output(ostream &os) const { return val.output(os); }
  friend ostream &operator<<(ostream &os, const wbox &b) {
    return b.output(os);
  }
};

template <typename T, int D> struct wregion : vregion<T> {
  region<T, D> val;

  wregion(const wregion &r) = default;
  wregion(wregion &&r) = default;
  wregion &operator=(const wregion &r) = default;
  wregion &operator=(wregion &&r) = default;
  unique_ptr<vregion<T>> copy() const {
    return unique_ptr<vregion<T>>(new wregion(*this));
  }

  wregion(const region<T, D> &r) : val(r) {}
  wregion(region<T, D> &&r) : val(std::move(r)) {}

  wregion() = default;
  wregion(const wbox<T, D> &b) : val(b.val) {}
  wregion(const vector<unique_ptr<vbox<T>>> &bs) {
    vector<box<T, D>> rs;
    for (const auto &b : bs)
      rs.push_back(dynamic_cast<const wbox<T, D> &>(b).val);
    val = region<T, D>(rs);
  }
  operator vector<unique_ptr<vbox<T>>>() const {
    vector<unique_ptr<vbox<T>>> bs;
    for (const auto &b : val.boxes)
      bs.push_back(make_unique<wbox<T, D>>(b));
    return bs;
  }
  template <typename U> wregion(const wregion<U, D> &p) : val(p.val) {}

  int rank() const { return D; }

  // Predicates
  bool invariant() const { return val.invariant(); }
  bool empty() const { return val.empty(); }
  typedef typename vregion<T>::prod_t prod_t;
  prod_t size() const { return val.size(); }

  // Set operations
  unique_ptr<vbox<T>> bounding_box() const {
    return make_unique<wbox<T, D>>(val.bounding_box());
  }
  unique_ptr<vregion<T>> operator&(const vbox<T> &b) const {
    return make_unique<wregion>(val & dynamic_cast<const wbox<T, D> &>(b).val);
  }
  unique_ptr<vregion<T>> operator&(const vregion<T> &r) const {
    return make_unique<wregion>(val & dynamic_cast<const wregion &>(r).val);
  }
  unique_ptr<vregion<T>> operator-(const vbox<T> &b) const {
    return make_unique<wregion>(val - dynamic_cast<const wbox<T, D> &>(b).val);
  }
  unique_ptr<vregion<T>> operator-(const vregion<T> &r) const {
    return make_unique<wregion>(val - dynamic_cast<const wregion &>(r).val);
  }
  unique_ptr<vregion<T>> operator|(const vbox<T> &b) const {
    return make_unique<wregion>(val | dynamic_cast<const wbox<T, D> &>(b).val);
  }
  unique_ptr<vregion<T>> operator|(const vregion<T> &r) const {
    return make_unique<wregion>(val | dynamic_cast<const wregion &>(r).val);
  }
  unique_ptr<vregion<T>> operator^(const vbox<T> &b) const {
    return make_unique<wregion>(val ^ dynamic_cast<const wbox<T, D> &>(b).val);
  }
  unique_ptr<vregion<T>> operator^(const vregion<T> &r) const {
    return make_unique<wregion>(val ^ dynamic_cast<const wregion &>(r).val);
  }

  // Set comparison operators
  bool contains(const vpoint<T> &p) const {
    return val.contains(dynamic_cast<const wpoint<T, D> &>(p).val);
  }
  bool isdisjoint(const vbox<T> &b) const {
    return val.isdisjoint(dynamic_cast<const wbox<T, D> &>(b).val);
  }
  bool isdisjoint(const vregion<T> &r) const {
    return val.isdisjoint(dynamic_cast<const wregion &>(r).val);
  }

  // Comparison operators
  bool operator<=(const vregion<T> &r) const {
    return val <= dynamic_cast<const wregion &>(r).val;
  }
  bool operator>=(const vregion<T> &r) const {
    return val >= dynamic_cast<const wregion &>(r).val;
  }
  bool operator<(const vregion<T> &r) const {
    return val < dynamic_cast<const wregion &>(r).val;
  }
  bool operator>(const vregion<T> &r) const {
    return val > dynamic_cast<const wregion &>(r).val;
  }
  bool operator==(const vregion<T> &r) const {
    return val == dynamic_cast<const wregion &>(r).val;
  }
  bool operator!=(const vregion<T> &r) const {
    return val != dynamic_cast<const wregion &>(r).val;
  }

  bool less(const vregion<T> &r) const {
    return val.less(dynamic_cast<const wregion &>(r).val);
  }

  // Output
  ostream &output(ostream &os) const { return val.output(os); }
  friend ostream &operator<<(ostream &os, const wregion &r) {
    return r.output(os);
  }
};

////////////////////////////////////////////////////////////////////////////////

// Dispatching functions (replacements for constructors)

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

template <typename T> unique_ptr<vpoint<T>> vpoint<T>::make(int d, const T &x) {
  switch (d) {
  case 0:
    return make_unique<wpoint<T, 0>>(x);
  case 1:
    return make_unique<wpoint<T, 1>>(x);
  case 2:
    return make_unique<wpoint<T, 2>>(x);
  case 3:
    return make_unique<wpoint<T, 3>>(x);
  case 4:
    return make_unique<wpoint<T, 4>>(x);
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

template <typename T>
template <typename U>
unique_ptr<vpoint<T>> vpoint<T>::make(const vpoint<U> &p) {
  switch (p.rank()) {
  case 0:
    return make_unique<wpoint<T, 0>>(dynamic_cast<const wpoint<U, 0> &>(p));
  case 1:
    return make_unique<wpoint<T, 1>>(dynamic_cast<const wpoint<U, 1> &>(p));
  case 2:
    return make_unique<wpoint<T, 2>>(dynamic_cast<const wpoint<U, 2> &>(p));
  case 3:
    return make_unique<wpoint<T, 3>>(dynamic_cast<const wpoint<U, 3> &>(p));
  case 4:
    return make_unique<wpoint<T, 4>>(dynamic_cast<const wpoint<U, 4> &>(p));
  default:
    assert(0);
  }
}

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
unique_ptr<vbox<T>> vbox<T>::make(const vpoint<T> &lo, const vpoint<T> &hi) {
  switch (lo.rank()) {
  case 0:
    return make_unique<wbox<T, 0>>(dynamic_cast<const wpoint<T, 0> &>(lo),
                                   dynamic_cast<const wpoint<T, 0> &>(hi));
  case 1:
    return make_unique<wbox<T, 1>>(dynamic_cast<const wpoint<T, 1> &>(lo),
                                   dynamic_cast<const wpoint<T, 1> &>(hi));
  case 2:
    return make_unique<wbox<T, 2>>(dynamic_cast<const wpoint<T, 2> &>(lo),
                                   dynamic_cast<const wpoint<T, 2> &>(hi));
  case 3:
    return make_unique<wbox<T, 3>>(dynamic_cast<const wpoint<T, 3> &>(lo),
                                   dynamic_cast<const wpoint<T, 3> &>(hi));
  case 4:
    return make_unique<wbox<T, 4>>(dynamic_cast<const wpoint<T, 4> &>(lo),
                                   dynamic_cast<const wpoint<T, 4> &>(hi));
  default:
    assert(0);
  }
}

template <typename T>
template <typename U>
unique_ptr<vbox<T>> vbox<T>::make(const vbox<U> &b) {
  switch (b.rank()) {
  case 0:
    return make_unique<wbox<T, 0>>(dynamic_cast<const wbox<U, 0> &>(b));
  case 1:
    return make_unique<wbox<T, 1>>(dynamic_cast<const wbox<U, 1> &>(b));
  case 2:
    return make_unique<wbox<T, 2>>(dynamic_cast<const wbox<U, 2> &>(b));
  case 3:
    return make_unique<wbox<T, 3>>(dynamic_cast<const wbox<U, 3> &>(b));
  case 4:
    return make_unique<wbox<T, 4>>(dynamic_cast<const wbox<U, 4> &>(b));
  default:
    assert(0);
  }
}

template <typename T> unique_ptr<vregion<T>> vregion<T>::make(int d) {
  switch (d) {
  case 0:
    return make_unique<wregion<T, 0>>();
  case 1:
    return make_unique<wregion<T, 1>>();
  case 2:
    return make_unique<wregion<T, 2>>();
  case 3:
    return make_unique<wregion<T, 3>>();
  case 4:
    return make_unique<wregion<T, 4>>();
  default:
    assert(0);
  }
}

template <typename T>
unique_ptr<vregion<T>> vregion<T>::make(const vbox<T> &b) {
  switch (b.rank()) {
  case 0:
    return make_unique<wregion<T, 0>>(dynamic_cast<const wbox<T, 0> &>(b));
  case 1:
    return make_unique<wregion<T, 1>>(dynamic_cast<const wbox<T, 1> &>(b));
  case 2:
    return make_unique<wregion<T, 2>>(dynamic_cast<const wbox<T, 2> &>(b));
  case 3:
    return make_unique<wregion<T, 3>>(dynamic_cast<const wbox<T, 3> &>(b));
  case 4:
    return make_unique<wregion<T, 4>>(dynamic_cast<const wbox<T, 4> &>(b));
  default:
    assert(0);
  }
}

template <typename T>
unique_ptr<vregion<T>> vregion<T>::make(const vector<unique_ptr<vbox<T>>> &bs) {
  if (bs.empty())
    return nullptr;
  switch (bs[0]->rank()) {
  case 0: {
    vector<box<T, 0>> rs;
    for (const auto &b : bs)
      rs.push_back(dynamic_cast<const wbox<T, 0> &>(*b).val);
    return make_unique<wregion<T, 0>>(rs);
  }
  case 1: {
    vector<box<T, 1>> rs;
    for (const auto &b : bs)
      rs.push_back(dynamic_cast<const wbox<T, 1> &>(*b).val);
    return make_unique<wregion<T, 1>>(rs);
  }
  case 2: {
    vector<box<T, 2>> rs;
    for (const auto &b : bs)
      rs.push_back(dynamic_cast<const wbox<T, 2> &>(*b).val);
    return make_unique<wregion<T, 2>>(rs);
  }
  case 3: {
    vector<box<T, 3>> rs;
    for (const auto &b : bs)
      rs.push_back(dynamic_cast<const wbox<T, 3> &>(*b).val);
    return make_unique<wregion<T, 3>>(rs);
  }
  case 4: {
    vector<box<T, 4>> rs;
    for (const auto &b : bs)
      rs.push_back(dynamic_cast<const wbox<T, 4> &>(*b).val);
    return make_unique<wregion<T, 4>>(rs);
  }
  default:
    assert(0);
  }
}

template <typename T>
template <typename U>
unique_ptr<vregion<T>> vregion<T>::make(const vregion<U> &r) {
  switch (r.rank()) {
  case 0:
    return make_unique<wregion<T, 0>>(dynamic_cast<const wregion<U, 0> &>(r));
  case 1:
    return make_unique<wregion<T, 1>>(dynamic_cast<const wregion<U, 1> &>(r));
  case 2:
    return make_unique<wregion<T, 2>>(dynamic_cast<const wregion<U, 2> &>(r));
  case 3:
    return make_unique<wregion<T, 3>>(dynamic_cast<const wregion<U, 3> &>(r));
  case 4:
    return make_unique<wregion<T, 4>>(dynamic_cast<const wregion<U, 4> &>(r));
  default:
    assert(0);
  }
}

////////////////////////////////////////////////////////////////////////////////

// Dimension-independent classes (hiding the pointers)

template <typename T> struct dpoint {
  unique_ptr<vpoint<T>> val;

  dpoint() = default;

  dpoint(const dpoint &p) {
    if (p.val)
      val = p.val.copy();
  }
  dpoint(dpoint &&p) = default;
  dpoint &operator=(const dpoint &p) {
    if (p.val)
      val = p.val->copy();
    else
      val = nullptr;
    return *this;
  }
  dpoint &operator=(dpoint &&p) = default;

  template <int D>
  dpoint(const point<T, D> &p)
      : val(make_unique<wpoint<T, D>>(p)) {}
  dpoint(const vpoint<T> &p) : val(p.copy()) {}
  dpoint(const unique_ptr<vpoint<T>> &val) {
    if (val)
      this->val = val->copy();
  }
  dpoint(unique_ptr<vpoint<T>> &&val) : val(std::move(val)) {}

  explicit dpoint(int d) : val(vpoint<T>::make(d)) {}
  dpoint(int d, const T &x) : val(vpoint<T>::make(d, x)) {}
  template <size_t D>
  dpoint(const array<T, D> &p)
      : val(make_unique<wpoint<T, D>>(p)) {}
  dpoint(const vector<T> &p) : val(vpoint<T>::make(p)) {}
  operator vector<T>() const { return vector<T>(*val); }
  template <typename U> dpoint(const dpoint<U> &p) {
    if (p.val)
      val = vpoint<T>::make(*p.val);
  }

  bool valid() const { return bool(val); }
  void reset() { val.reset(); }
  int rank() const { return val->rank(); }

  // Unary operators
  dpoint operator+() const { return dpoint(+*val); }
  dpoint operator-() const { return dpoint(-*val); }
  dpoint operator~() const { return dpoint(~*val); }
  dpoint<bool> operator!() const { return dpoint<bool>(!*val); }

  // Assignment operators
  dpoint &operator+=(const dpoint &p) {
    *val += *p.val;
    return *this;
  }
  dpoint &operator-=(const dpoint &p) {
    *val -= *p.val;
    return *this;
  }
  dpoint &operator*=(const dpoint &p) {
    *val *= *p.val;
    return *this;
  }
  dpoint &operator/=(const dpoint &p) {
    *val /= *p.val;
    return *this;
  }
  dpoint &operator%=(const dpoint &p) {
    *val %= *p.val;
    return *this;
  }
  dpoint &operator&=(const dpoint &p) {
    *val &= *p.val;
    return *this;
  }
  dpoint &operator|=(const dpoint &p) {
    *val |= *p.val;
    return *this;
  }
  dpoint &operator^=(const dpoint &p) {
    *val ^= *p.val;
    return *this;
  }

  // Binary operators
  dpoint operator+(const dpoint &p) const { return dpoint(*val + *p.val); }
  dpoint operator-(const dpoint &p) const { return dpoint(*val - *p.val); }
  dpoint operator*(const dpoint &p) const { return dpoint(*val * *p.val); }
  dpoint operator/(const dpoint &p) const { return dpoint(*val / *p.val); }
  dpoint operator%(const dpoint &p) const { return dpoint(*val % *p.val); }
  dpoint operator&(const dpoint &p) const { return dpoint(*val & *p.val); }
  dpoint operator|(const dpoint &p) const { return dpoint(*val | *p.val); }
  dpoint operator^(const dpoint &p) const { return dpoint(*val ^ *p.val); }
  dpoint<bool> operator&&(const dpoint &p) const {
    return dpoint<bool>(*val && *p.val);
  }
  dpoint<bool> operator||(const dpoint &p) const {
    return dpoint<bool>(*val || *p.val);
  }

  // Unary functions
  dpoint abs() const { return dpoint(val->abs()); }

  // Binary functions
  dpoint min(const dpoint &p) const { return dpoint(val->min(*p.val)); }
  dpoint max(const dpoint &p) const { return dpoint(val->max(*p.val)); }

  // Comparison operators
  dpoint<bool> operator==(const dpoint &p) const {
    return dpoint<bool>(*val == *p.val);
  }
  dpoint<bool> operator!=(const dpoint &p) const {
    return dpoint<bool>(*val != *p.val);
  }
  dpoint<bool> operator<(const dpoint &p) const {
    return dpoint<bool>(*val < *p.val);
  }
  dpoint<bool> operator>(const dpoint &p) const {
    return dpoint<bool>(*val > *p.val);
  }
  dpoint<bool> operator<=(const dpoint &p) const {
    return dpoint<bool>(*val <= *p.val);
  }
  dpoint<bool> operator>=(const dpoint &p) const {
    return dpoint<bool>(*val >= *p.val);
  }

  bool equal_to(const dpoint &p) const { return val->equal_to(*p.val); }
  bool less(const dpoint &p) const { return val->less(*p.val); }

  // Reductions
  bool all() const { return val->all(); }
  bool any() const { return val->any(); }
  T minval() const { return val->minval(); }
  T maxval() const { return val->maxval(); }
  T sum() const { return val->sum(); }
  typedef typename vpoint<T>::prod_t prod_t;
  prod_t prod() const { return val->prod(); }

  // Output
  ostream &output(ostream &os) const {
    if (!val)
      return os << "dpoint()";
    return val->output(os);
  }
  friend ostream &operator<<(ostream &os, const dpoint &p) {
    return p.output(os);
  }
};

// Unary functions
template <typename T> dpoint<T> abs(const dpoint<T> &p) { return p.abs(); }

// Binary functions
template <typename T> dpoint<T> min(const dpoint<T> &p, const dpoint<T> &q) {
  return p.min(q);
}
template <typename T> dpoint<T> max(const dpoint<T> &p, const dpoint<T> &q) {
  return p.max(q);
}

// Reductions
template <typename T> bool all(const dpoint<T> &p) { return p.all(); }
template <typename T> bool any(const dpoint<T> &p) { return p.any(); }
template <typename T> T minval(const dpoint<T> &p) { return p.minval(); }
template <typename T> T maxval(const dpoint<T> &p) { return p.maxval(); }
template <typename T> T sum(const dpoint<T> &p) { return p.sum(); }
template <typename T> typename dpoint<T>::prod_t prod(const dpoint<T> &p) {
  return p.prod();
}
}

namespace std {
template <typename T> struct equal_to<RegionCalculus::dpoint<T>> {
  bool operator()(const RegionCalculus::dpoint<T> &p,
                  const RegionCalculus::dpoint<T> &q) const {
    return p.equal_to(q);
  }
};

template <typename T> struct less<RegionCalculus::dpoint<T>> {
  bool operator()(const RegionCalculus::dpoint<T> &p,
                  const RegionCalculus::dpoint<T> &q) const {
    return p.less(q);
  }
};
}

namespace RegionCalculus {
template <typename T> struct dbox {
  unique_ptr<vbox<T>> val;

  dbox() = default;

  dbox(const dbox &b) {
    if (b.val)
      val = b.val->copy();
  }
  dbox(dbox &&b) = default;
  dbox &operator=(const dbox &b) {
    if (b.val)
      val = b.val->copy();
    else
      val = nullptr;
    return *this;
  }
  dbox &operator=(dbox &&b) = default;

  template <int D> dbox(const box<T, D> &b) : val(make_unique<wbox<T, D>>(b)) {}
  dbox(const vbox<T> &b) : val(b.copy()) {}
  dbox(const unique_ptr<vbox<T>> &val) {
    if (val)
      this->val = val->copy();
  }
  dbox(unique_ptr<vbox<T>> &&val) : val(std::move(val)) {}

  explicit dbox(int d) : val(vbox<T>::make(d)) {}
  dbox(const dpoint<T> &lo, const dpoint<T> &hi)
      : val(vbox<T>::make(*lo.val, *hi.val)) {}
  template <typename U> dbox(const dbox<U> &p) {
    if (p.val)
      val = vbox<T>::make(*p.val);
  }

  bool valid() const { return bool(val); }
  void reset() { val.reset(); }
  int rank() const { return val->rank(); }

  // Predicates
  bool empty() const { return val->empty(); }
  dpoint<T> lower() const { return dpoint<T>(val->lower()); }
  dpoint<T> upper() const { return dpoint<T>(val->upper()); }
  dpoint<T> shape() const { return dpoint<T>(val->shape()); }
  typedef typename vbox<T>::prod_t prod_t;
  prod_t size() const { return val->size(); }

  // Shift and scale operators
  dbox &operator>>=(const dpoint<T> &p) {
    *val >>= *p.val;
    return *this;
  }
  dbox &operator<<=(const dpoint<T> &p) {
    *val <<= *p.val;
    return *this;
  }
  dbox &operator*=(const dpoint<T> &p) {
    *val *= *p.val;
    return *this;
  }
  dbox operator>>(const dpoint<T> &p) const { return dbox(*val >> *p.val); }
  dbox operator<<(const dpoint<T> &p) const { return dbox(*val << *p.val); }
  dbox operator*(const dpoint<T> &p) const { return dbox(*val * *p.val); }

  // Comparison operators
  bool operator==(const dbox &b) const { return *val == *b.val; }
  bool operator!=(const dbox &b) const { return *val != *b.val; }
  bool less(const dbox &b) const { return val->less(*b.val); }

  // Set comparison operators
  bool contains(const dpoint<T> &p) const { return val->contains(*p.val); }
  bool isdisjoint(const dbox &b) const { return val->isdisjoint(*b.val); }
  bool operator<=(const dbox &b) const { return *val <= *b.val; }
  bool operator>=(const dbox &b) const { return *val >= *b.val; }
  bool operator<(const dbox &b) const { return *val < *b.val; }
  bool operator>(const dbox &b) const { return *val > *b.val; }
  bool issubset(const dbox &b) const { return *this <= b; }
  bool issuperset(const dbox &b) const { return *this >= b; }
  bool is_strict_subset(const dbox &b) const { return *this < b; }
  bool is_strict_superset(const dbox &b) const { return *this > b; }

  // Set operations
  dbox bounding_box(const dbox &b) const {
    return dbox(val->bounding_box(*b.val));
  }
  dbox operator&(const dbox &b) const { return dbox(*val & *b.val); }
  // dbox operator-(const dbox &b) const { return dbox(*val - *b.val); }
  // dbox operator|(const dbox &b) const { return dbox(*val | *b.val); }
  // dbox operator^(const dbox &b) const { return dbox(*val ^ *b.val); }
  dbox intersection(const dbox &b) const { return *this & b; }
  // dbox difference(const dbox &b) const { return *this - b; }
  // dbox setunion(const dbox &b) const { return *this | b; }
  // dbox symmetric_difference(const dbox &b) const { return *this ^ b; }

  // Output
  ostream &output(ostream &os) const {
    if (!val)
      return os << "dbox()";
    return val->output(os);
  }
  friend ostream &operator<<(ostream &os, const dbox &b) {
    return b.output(os);
  }
};
}

namespace std {
template <typename T> struct less<RegionCalculus::dbox<T>> {
  bool operator()(const RegionCalculus::dbox<T> &p,
                  const RegionCalculus::dbox<T> &q) const {
    return p.less(q);
  }
};
}

namespace RegionCalculus {
template <typename T> struct dregion {
  unique_ptr<vregion<T>> val;

  dregion() = default;

  dregion(const dregion &r) {
    if (r.val)
      val = r.val->copy();
  }
  // dregion(dregion &&r) = default;
  dregion &operator=(const dregion &r) {
    if (r.val)
      val = r.val->copy();
    else
      val = nullptr;
    return *this;
  }
  // dregion &operator=(dregion &&r) = default;

  template <int D>
  dregion(const region<T, D> &r)
      : val(make_unique<wregion<T, D>>(r)) {}
  dregion(const vregion<T> &r) : val(r.copy()) {}
  dregion(const unique_ptr<vregion<T>> &val) {
    if (val)
      this->val = val->copy();
  }
  // dregion(unique_ptr<vregion<T>> &&val) : val(std::move(val)) {}

  explicit dregion(int d) : val(vregion<T>::make(d)) {}
  dregion(const dbox<T> &b) : val(vregion<T>::make(*b.val)) {}
  dregion(const vector<dbox<T>> &bs) {
    vector<unique_ptr<vbox<T>>> rs;
    for (const auto &b : bs)
      rs.push_back(b.val->copy());
    val = vregion<T>::make(rs);
  }
  // dregion(vector<dbox<T>> &&bs) {
  //   vector<unique_ptr<vbox<T>>> rs;
  //   for (auto &b : bs)
  //     rs.push_back(std::move(b.val));
  //   val = vregion<T>::make(rs);
  // }
  operator vector<dbox<T>>() const {
    vector<unique_ptr<vbox<T>>> bs(*val);
    vector<dbox<T>> rs;
    for (auto &b : bs)
      rs.push_back(dbox<T>(std::move(b)));
    return rs;
  }
  template <typename U> dregion(const dregion<U> &p) {
    if (p.val)
      val = vregion<T>::make(*p.val);
  }

  bool valid() const { return bool(val); }
  void reset() { val.reset(); }
  int rank() const { return val->rank(); }

  // Predicates
  bool invariant() const { return val->invariant(); }
  bool empty() const { return val->empty(); }
  typedef typename vregion<T>::prod_t prod_t;
  prod_t size() const { return val->size(); }

  // Set operations
  dregion bounding_box() const { return dregion(val->bounding_box()); }
  dregion operator&(const dbox<T> &b) const { return dregion(*val & *b.val); }
  dregion operator&(const dregion &r) const { return dregion(*val & *r.val); }
  dregion operator-(const dbox<T> &b) const { return dregion(*val - *b.val); }
  dregion operator-(const dregion &r) const { return dregion(*val - *r.val); }
  dregion operator|(const dbox<T> &b) const { return dregion(*val | *b.val); }
  dregion operator|(const dregion &r) const { return dregion(*val | *r.val); }
  dregion operator^(const dbox<T> &b) const { return dregion(*val ^ *b.val); }
  dregion operator^(const dregion &r) const { return dregion(*val ^ *r.val); }
  dregion intersection(const dbox<T> &b) const { return *this & b; }
  dregion intersection(const dregion &r) const { return *this & r; }
  dregion difference(const dbox<T> &b) const { return *this - b; }
  dregion difference(const dregion &r) const { return *this - r; }
  dregion setunion(const dbox<T> &b) const { return *this | b; }
  dregion setunion(const dregion &r) const { return *this | r; }
  dregion symmetric_difference(const dbox<T> &b) const { return *this ^ b; }
  dregion symmetric_difference(const dregion &r) const { return *this ^ r; }

  // Set comparison operators
  bool contains(const dpoint<T> &p) const { return val->contains(*p.val); }
  bool isdisjoint(const dbox<T> &b) const { return val->isdisjoint(*b.val); }
  bool isdisjoint(const dregion &r) const { return val->isdisjoint(*r.val); }

  // Comparison operators
  bool operator<=(const dregion &r) const { return *val <= *r.val; }
  bool operator>=(const dregion &r) const { return *val >= *r.val; }
  bool operator<(const dregion &r) const { return *val < *r.val; }
  bool operator>(const dregion &r) const { return *val > *r.val; }
  bool issubset(const dregion &r) const { return *this <= r; }
  bool issuperset(const dregion &r) const { return *this >= r; }
  bool is_strict_subset(const dregion &r) const { return *this < r; }
  bool is_strict_superset(const dregion &r) const { return *this > r; }
  bool operator==(const dregion &r) const { return *val == *r.val; }
  bool operator!=(const dregion &r) const { return *val != *r.val; }

  bool less(const dregion &r) const { return val->less(*r.val); }

  // Output
  ostream &output(ostream &os) const {
    if (!val)
      return os << "dregion()";
    return val->output(os);
  }
  friend ostream &operator<<(ostream &os, const dregion &r) {
    return r.output(os);
  }
};
}

namespace std {
template <typename T> struct less<RegionCalculus::dregion<T>> {
  bool operator()(const RegionCalculus::dregion<T> &p,
                  const RegionCalculus::dregion<T> &q) const {
    return p.less(q);
  }
};
}

#endif // REGIONCALCULUS_HPP
