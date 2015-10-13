#ifndef DISCRETIZATIONBLOCK_HPP
#define DISCRETIZATIONBLOCK_HPP

#include "Common.hpp"
#include "Discretization.hpp"

#include <cassert>
#include <iostream>
#include <string>

namespace SimulationIO {

struct DiscretizationBlock : Common {
  // Discretization of a certain region, represented by contiguous data
  Discretization *discretization;
  // bounding box? in terms of coordinates?
  // connectivity? neighbouring blocks?
  // overlaps?

  virtual bool invariant() const {
    return Common::invariant() && bool(discretization);
  }

  DiscretizationBlock() = delete;
  DiscretizationBlock(const DiscretizationBlock &) = delete;
  DiscretizationBlock(DiscretizationBlock &&) = delete;
  DiscretizationBlock &operator=(const DiscretizationBlock &) = delete;
  DiscretizationBlock &operator=(DiscretizationBlock &&) = delete;

private:
  friend class Discretization;
  DiscretizationBlock(const string &name, Discretization *discretization)
      : Common(name), discretization(discretization) {}
  DiscretizationBlock(const H5::CommonFG &loc, const string &entry,
                      Discretization *discretization);

public:
  virtual ~DiscretizationBlock() { assert(0); }

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os,
                             const DiscretizationBlock &discretizationblock) {
    return discretizationblock.output(os);
  }
  virtual void write(const H5::CommonFG &loc,
                     const H5::H5Location &parent) const;
};
}

#define DISCRETIZATIONBLOCK_HPP_DONE
#endif // #ifndef DISCRETIZATIONBLOCK_HPP
#ifndef DISCRETIZATIONBLOCK_HPP_DONE
#error "Cyclic include depencency"
#endif
