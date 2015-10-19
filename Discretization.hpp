#ifndef DISCRETIZATION_HPP
#define DISCRETIZATION_HPP

#include "Common.hpp"
#include "Manifold.hpp"

#include <H5Cpp.h>

#include <iostream>
#include <map>
#include <memory>
#include <string>

namespace SimulationIO {

using std::make_shared;
using std::map;
using std::ostream;
using std::shared_ptr;
using std::string;

struct DiscreteField;
struct DiscretizationBlock;

struct Discretization : Common, std::enable_shared_from_this<Discretization> {
  shared_ptr<Manifold> manifold;                                     // parent
  map<string, shared_ptr<DiscretizationBlock>> discretizationblocks; // children
  NoBackLink<shared_ptr<DiscreteField>> discretefields;

  virtual bool invariant() const {
    return Common::invariant() && bool(manifold) &&
           manifold->discretizations.count(name) &&
           manifold->discretizations.at(name).get() == this;
  }

  Discretization() = delete;
  Discretization(const Discretization &) = delete;
  Discretization(Discretization &&) = delete;
  Discretization &operator=(const Discretization &) = delete;
  Discretization &operator=(Discretization &&) = delete;

  friend class Manifold;
  Discretization(hidden, const string &name,
                 const shared_ptr<Manifold> &manifold)
      : Common(name), manifold(manifold) {}
  Discretization(hidden) : Common(hidden()) {}

private:
  static shared_ptr<Discretization>
  create(const string &name, const shared_ptr<Manifold> &manifold) {
    return make_shared<Discretization>(hidden(), name, manifold);
  }
  static shared_ptr<Discretization>
  create(const H5::CommonFG &loc, const string &entry,
         const shared_ptr<Manifold> &manifold) {
    auto discretization = make_shared<Discretization>(hidden());
    discretization->read(loc, entry, manifold);
    return discretization;
  }
  void read(const H5::CommonFG &loc, const string &entry,
            const shared_ptr<Manifold> &manifold);

public:
  virtual ~Discretization() {}

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os,
                             const Discretization &discretization) {
    return discretization.output(os);
  }
  virtual void write(const H5::CommonFG &loc,
                     const H5::H5Location &parent) const;

  shared_ptr<DiscretizationBlock> createDiscretizationBlock(const string &name);
  shared_ptr<DiscretizationBlock>
  createDiscretizationBlock(const H5::CommonFG &loc, const string &entry);

  void noinsert(const shared_ptr<DiscreteField> &discretefield) {}
};
}

#define DISCRETIZATION_HPP_DONE
#endif // #ifndef DISCRETIZATION_HPP
#ifndef DISCRETIZATION_HPP_DONE
#error "Cyclic include depencency"
#endif
