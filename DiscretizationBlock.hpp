#ifndef DISCRETIZATIONBLOCK_HPP
#define DISCRETIZATIONBLOCK_HPP

#include "Common.hpp"
#include "Discretization.hpp"

#include <H5Cpp.h>

#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace SimulationIO {

using std::make_shared;
using std::ostream;
using std::shared_ptr;
using std::string;
using std::vector;
using std::weak_ptr;

struct DiscreteFieldBlock;

struct DiscretizationBlock : Common,
                             std::enable_shared_from_this<DiscretizationBlock> {
  // Discretization of a certain region, represented by contiguous data
  weak_ptr<Discretization> discretization; // parent
  vector<hssize_t> offset;
  NoBackLink<weak_ptr<DiscreteFieldBlock>> discretefieldblocks;

  // bounding box? in terms of coordinates?
  // connectivity? neighbouring blocks?
  // overlaps?

  virtual bool invariant() const {
    return Common::invariant() && bool(discretization.lock()) &&
           discretization.lock()->discretizationblocks.count(name) &&
           discretization.lock()->discretizationblocks.at(name).get() == this &&
           (offset.empty() ||
            int(offset.size()) ==
                discretization.lock()->manifold.lock()->dimension);
  }

  DiscretizationBlock() = delete;
  DiscretizationBlock(const DiscretizationBlock &) = delete;
  DiscretizationBlock(DiscretizationBlock &&) = delete;
  DiscretizationBlock &operator=(const DiscretizationBlock &) = delete;
  DiscretizationBlock &operator=(DiscretizationBlock &&) = delete;

  friend struct Discretization;
  DiscretizationBlock(hidden, const string &name,
                      const shared_ptr<Discretization> &discretization)
      : Common(name), discretization(discretization) {}
  DiscretizationBlock(hidden) : Common(hidden()) {}

private:
  static shared_ptr<DiscretizationBlock>
  create(const string &name, const shared_ptr<Discretization> &discretization) {
    return make_shared<DiscretizationBlock>(hidden(), name, discretization);
  }
  static shared_ptr<DiscretizationBlock>
  create(const H5::CommonFG &loc, const string &entry,
         const shared_ptr<Discretization> &discretization) {
    auto discretizationblock = make_shared<DiscretizationBlock>(hidden());
    discretizationblock->read(loc, entry, discretization);
    return discretizationblock;
  }
  void read(const H5::CommonFG &loc, const string &entry,
            const shared_ptr<Discretization> &discretization);

public:
  virtual ~DiscretizationBlock() {}

  void setOffset() { offset.clear(); }
  void setOffset(const std::vector<hssize_t> &offset_) {
    assert(int(offset_.size()) ==
           discretization.lock()->manifold.lock()->dimension);
    offset = offset_;
  }

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os,
                             const DiscretizationBlock &discretizationblock) {
    return discretizationblock.output(os);
  }
  virtual void write(const H5::CommonFG &loc,
                     const H5::H5Location &parent) const;

private:
  friend struct DiscreteFieldBlock;
  void noinsert(const shared_ptr<DiscreteFieldBlock> &discretefieldblock) {}
};
}

#define DISCRETIZATIONBLOCK_HPP_DONE
#endif // #ifndef DISCRETIZATIONBLOCK_HPP
#ifndef DISCRETIZATIONBLOCK_HPP_DONE
#error "Cyclic include depencency"
#endif
