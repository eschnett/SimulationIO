#ifndef DISCRETIZATION_HPP
#define DISCRETIZATION_HPP

#include "Common.hpp"
#include "Manifold.hpp"

#include <H5Cpp.h>

#include <cassert>
#include <iostream>
#include <map>
#include <string>

namespace SimulationIO {

using std::ostream;
using std::map;
using std::string;

struct DiscreteField;
struct DiscretizationBlock;

struct Discretization : Common {
  Manifold *manifold;                                      // parent
  map<string, DiscretizationBlock *> discretizationblocks; // children
  NoBackLink<DiscreteField *> discretefields;

  virtual bool invariant() const {
    return Common::invariant() && bool(manifold) &&
           manifold->discretizations.count(name) &&
           manifold->discretizations.at(name) == this;
  }

  Discretization() = delete;
  Discretization(const Discretization &) = delete;
  Discretization(Discretization &&) = delete;
  Discretization &operator=(const Discretization &) = delete;
  Discretization &operator=(Discretization &&) = delete;

private:
  friend class Manifold;
  Discretization(const string &name, Manifold *manifold)
      : Common(name), manifold(manifold) {}
  Discretization(const H5::CommonFG &loc, const string &entry,
                 Manifold *manifold);

public:
  virtual ~Discretization() { assert(0); }

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os,
                             const Discretization &discretization) {
    return discretization.output(os);
  }
  virtual void write(const H5::CommonFG &loc,
                     const H5::H5Location &parent) const;

  DiscretizationBlock *createDiscretizationBlock(const string &name);
  DiscretizationBlock *createDiscretizationBlock(const H5::CommonFG &loc,
                                                 const string &entry);

  void noinsert(DiscreteField *discretefield) {}
};
}

#define DISCRETIZATION_HPP_DONE
#endif // #ifndef DISCRETIZATION_HPP
#ifndef DISCRETIZATION_HPP_DONE
#error "Cyclic include depencency"
#endif
