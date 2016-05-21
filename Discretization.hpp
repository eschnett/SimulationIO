#ifndef DISCRETIZATION_HPP
#define DISCRETIZATION_HPP

#include "Common.hpp"
#include "Configuration.hpp"
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
using std::weak_ptr;

class DiscreteField;
class DiscretizationBlock;
class SubDiscretization;

class Discretization : public Common,
                       public std::enable_shared_from_this<Discretization> {
public:
  weak_ptr<Manifold> manifold;             // parent
  shared_ptr<Configuration> configuration; // with backlink
  map<string, shared_ptr<DiscretizationBlock>> discretizationblocks; // children
  map<string, weak_ptr<SubDiscretization>> child_discretizations;  // backlinks
  map<string, weak_ptr<SubDiscretization>> parent_discretizations; // backlinks
  NoBackLink<weak_ptr<DiscreteField>> discretefields;

  virtual bool invariant() const {
    return Common::invariant() && bool(manifold.lock()) &&
           manifold.lock()->discretizations().count(name) &&
           manifold.lock()->discretizations().at(name).get() == this &&
           bool(configuration) && configuration->discretizations.count(name) &&
           configuration->discretizations.at(name).lock().get() == this;
  }

  Discretization() = delete;
  Discretization(const Discretization &) = delete;
  Discretization(Discretization &&) = delete;
  Discretization &operator=(const Discretization &) = delete;
  Discretization &operator=(Discretization &&) = delete;

  friend class Manifold;
  Discretization(hidden, const string &name,
                 const shared_ptr<Manifold> &manifold,
                 const shared_ptr<Configuration> &configuration)
      : Common(name), manifold(manifold), configuration(configuration) {}
  Discretization(hidden) : Common(hidden()) {}

private:
  static shared_ptr<Discretization>
  create(const string &name, const shared_ptr<Manifold> &manifold,
         const shared_ptr<Configuration> &configuration) {
    auto discretization =
        make_shared<Discretization>(hidden(), name, manifold, configuration);
    configuration->insert(name, discretization);
    return discretization;
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
  readDiscretizationBlock(const H5::CommonFG &loc, const string &entry);

private:
  friend class SubDiscretization;
  void insertChild(const string &name,
                   const shared_ptr<SubDiscretization> &subdiscretization) {
    checked_emplace(child_discretizations, name, subdiscretization);
  }
  void insertParent(const string &name,
                    const shared_ptr<SubDiscretization> &subdiscretization) {
    checked_emplace(parent_discretizations, name, subdiscretization);
  }
  friend class DiscreteField;
  void noinsert(const shared_ptr<DiscreteField> &discretefield) {}
};
}

#define DISCRETIZATION_HPP_DONE
#endif // #ifndef DISCRETIZATION_HPP
#ifndef DISCRETIZATION_HPP_DONE
#error "Cyclic include depencency"
#endif
