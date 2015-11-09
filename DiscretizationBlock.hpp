#ifndef DISCRETIZATIONBLOCK_HPP
#define DISCRETIZATIONBLOCK_HPP

#include "Common.hpp"
#include "Discretization.hpp"
#include "RegionCalculus.hpp"

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

typedef RegionCalculus::dpoint<hssize_t> point_t;
typedef RegionCalculus::dbox<hssize_t> box_t;
typedef RegionCalculus::dregion<hssize_t> region_t;

struct DiscreteFieldBlock;

struct DiscretizationBlock : Common,
                             std::enable_shared_from_this<DiscretizationBlock> {
  // Discretization of a certain region, represented by contiguous data
  weak_ptr<Discretization> discretization; // parent
  box_t region;
  region_t active;
  NoBackLink<weak_ptr<DiscreteFieldBlock>> discretefieldblocks;

  // bounding box? in terms of coordinates?
  // connectivity? neighbouring blocks?
  // overlaps?

  virtual bool invariant() const {
    return Common::invariant() && bool(discretization.lock()) &&
           discretization.lock()->discretizationblocks.count(name) &&
           discretization.lock()->discretizationblocks.at(name).get() == this &&
           (!bool(region) ||
            (region.rank() ==
                 discretization.lock()->manifold.lock()->dimension &&
             !region.empty()));
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

  void setRegion() { region.reset(); }
  void setRegion(const box_t &region_) {
    assert(bool(region_) &&
           region_.rank() ==
               discretization.lock()->manifold.lock()->dimension &&
           !region_.empty());
    region = region_;
  }
  box_t getRegion() const { return region; }

  void setActive() { active.reset(); }
  void setActive(const region_t &active_) {
    assert(bool(active_) &&
           active_.rank() == discretization.lock()->manifold.lock()->dimension);
    active = active_;
  }
  region_t getActive() const { return active; }

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
