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

using namespace RegionCalculus;

using std::make_shared;
using std::ostream;
using std::shared_ptr;
using std::string;
using std::vector;
using std::weak_ptr;

class DiscreteFieldBlock;

class DiscretizationBlock
    : public Common,
      public std::enable_shared_from_this<DiscretizationBlock> {
  // Discretization of a certain region, represented by contiguous data
  weak_ptr<Discretization> m_discretization; // parent
  box_t m_box;
  region_t m_active;
  NoBackLink<weak_ptr<DiscreteFieldBlock>> m_discretefieldblocks;

public:
  shared_ptr<Discretization> discretization() const {
    return m_discretization.lock();
  }
  const box_t &box() const { return m_box; }
  const region_t &active() const { return m_active; }
  NoBackLink<weak_ptr<DiscreteFieldBlock>> discretefieldblocks() const {
    return m_discretefieldblocks;
  }

  // bounding box? in terms of coordinates?
  // connectivity? neighbouring blocks?
  // overlaps?

  virtual bool invariant() const {
    return Common::invariant() && bool(discretization()) &&
           discretization()->discretizationblocks().count(name()) &&
           discretization()->discretizationblocks().at(name()).get() == this &&
           (!box().valid() ||
            (box().rank() == discretization()->manifold()->dimension() &&
             !box().empty())) &&
           (!active().valid() ||
            active().rank() == discretization()->manifold()->dimension());
  }

  DiscretizationBlock() = delete;
  DiscretizationBlock(const DiscretizationBlock &) = delete;
  DiscretizationBlock(DiscretizationBlock &&) = delete;
  DiscretizationBlock &operator=(const DiscretizationBlock &) = delete;
  DiscretizationBlock &operator=(DiscretizationBlock &&) = delete;

  friend class Discretization;
  DiscretizationBlock(hidden, const string &name,
                      const shared_ptr<Discretization> &discretization)
      : Common(name), m_discretization(discretization) {}
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

  void merge(const shared_ptr<DiscretizationBlock> &discretizationblock);

  void setBox() { m_box.reset(); }
  void setBox(const box_t &box) {
    assert(box.valid() &&
           box.rank() == discretization()->manifold()->dimension() &&
           !box.empty());
    m_box = box;
  }
  box_t getBox() const { return m_box; }

  void setActive() { m_active.reset(); }
  void setActive(const region_t &active) {
    assert(active.valid() &&
           active.rank() == discretization()->manifold()->dimension());
    m_active = active;
  }
  region_t getActive() const { return m_active; }

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os,
                             const DiscretizationBlock &discretizationblock) {
    return discretizationblock.output(os);
  }
  virtual void write(const H5::CommonFG &loc,
                     const H5::H5Location &parent) const;

private:
  friend class DiscreteFieldBlock;
  void noinsert(const shared_ptr<DiscreteFieldBlock> &discretefieldblock) {}
};
}

#define DISCRETIZATIONBLOCK_HPP_DONE
#endif // #ifndef DISCRETIZATIONBLOCK_HPP
#ifndef DISCRETIZATIONBLOCK_HPP_DONE
#error "Cyclic include depencency"
#endif
