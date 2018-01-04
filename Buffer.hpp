#ifndef BUFFER_HPP
#define BUFFER_HPP

#include "RegionCalculus.hpp"

#include <cassert>
#include <limits>
#include <memory>
#include <vector>

namespace SimulationIO {

using std::enable_shared_from_this;
using std::make_shared;
using std::numeric_limits;
using std::shared_ptr;
using std::unique_ptr;
using std::vector;

#if 0
// A range is described by its minimum (inclusive), maximum (inclusive), and
// count (non-negative). Here we use double precision for all three fields, but
// other precisions are also possible. We use a floating point number to
// describe the count for uniformity.
struct range_t {
  double minimum, maximum, count;
};
#endif

class dlinearization_t;
class dconcatenation_t;
template <size_t D> class linearization_t;
template <size_t D> class concatenation_t;

class dlinearization_t {

public:
  virtual ~dlinearization_t() {}
  virtual int dim() const = 0;
  virtual RegionCalculus::box_t box() const = 0;
  virtual long long pos() const = 0;
};

template <size_t D> class linearization_t : public dlinearization_t {
  shared_ptr<concatenation_t<D>> m_concatenation;

public:
  typedef RegionCalculus::box<long long, D> box_t;

private:
  box_t m_box;
  long long m_pos;

public:
  virtual int dim() const { return D; }
  virtual RegionCalculus::box_t box() const {
    return RegionCalculus::box_t(m_box);
  }
  virtual long long pos() const { return m_pos; }

  linearization_t() = delete;
  linearization_t(const linearization_t &) = default;
  linearization_t(linearization_t &&) = default;
  linearization_t &operator=(const linearization_t &) = default;
  linearization_t &operator=(linearization_t &&) = default;

  linearization_t(const shared_ptr<concatenation_t<D>> &concatenation,
                  const box_t &box, long long pos)
      : m_concatenation(concatenation) {
    // assert(box.valid());
    // assert(box.rank() == D);
    assert(!box.empty());
    assert(pos >= 0);
    m_box = box;
    m_pos = pos;
  }
  // box_t box() const { return m_box; }
  // long long pos() const { return m_pos; }
};

class dconcatenation_t {
public:
  virtual ~dconcatenation_t() {}
  virtual int dim() const = 0;
  static shared_ptr<dconcatenation_t> make(int dim);
  virtual unique_ptr<dlinearization_t>
  push_back(const RegionCalculus::box_t &box) = 0;
};

template <size_t D>
class concatenation_t : public enable_shared_from_this<concatenation_t<D>>,
                        public dconcatenation_t {
public:
  typedef RegionCalculus::box<long long, D> box_t;

private:
  vector<linearization_t<D>> linearizations;
  long long next;

public:
  virtual int dim() const { return D; }

  concatenation_t() : next(0) {}
  concatenation_t(const concatenation_t &) = default;
  concatenation_t(concatenation_t &&) = default;
  concatenation_t &operator=(const concatenation_t &) = default;
  concatenation_t &operator=(concatenation_t &&) = default;

  linearization_t<D> push_back(const box_t &box) {
    // assert(box.valid());
    // assert(box.rank() == D);
    assert(!box.empty());
    long long size = box.size();
    assert(size <= numeric_limits<long long>::max() - next);
    auto linearization =
        linearization_t<D>(this->shared_from_this(), box, next);
    linearizations.push_back(linearization);
    next += size;
    return linearization;
  }

  virtual unique_ptr<dlinearization_t>
  push_back(const RegionCalculus::box_t &box) {
    return make_unique1<linearization_t<D>>(push_back(box_t(box)));
  }
};
} // namespace SimulationIO

#define BUFFER_HPP_DONE
#endif // #ifndef BUFFER_HPP
#ifndef BUFFER_HPP_DONE
#error "Cyclic include depencency"
#endif
