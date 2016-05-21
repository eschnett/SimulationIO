#ifndef SUBDISCRETIZATION_HPP
#define SUBDISCRETIZATION_HPP

#include "Common.hpp"
#include "Discretization.hpp"
#include "Manifold.hpp"

#include <H5Cpp.h>

#include <cmath>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace SimulationIO {

using std::make_shared;
using std::map;
using std::ostream;
using std::pair;
using std::shared_ptr;
using std::string;
using std::vector;
using std::weak_ptr;

class SubDiscretization
    : public Common,
      public std::enable_shared_from_this<SubDiscretization> {
public:
  weak_ptr<Manifold> manifold;
  shared_ptr<Discretization> parent_discretization;
  shared_ptr<Discretization> child_discretization;
  vector<double> factor;
  vector<double> offset;
  // child_idx = factor * parent_idx - offset
  // AMR refinement: factor=2, offset=0
  // cell from vertex: factor=1, offset=1/2

  vector<double> child2parent(const vector<double> &child_idx) const {
    vector<double> parent_idx(child_idx.size());
    for (int d = 0; d < int(parent_idx.size()); ++d)
      parent_idx.at(d) = (child_idx.at(d) + offset.at(d)) / factor.at(d);
    return parent_idx;
  }
  vector<double> parent2child(const vector<double> &parent_idx) const {
    vector<double> child_idx(parent_idx.size());
    for (int d = 0; d < int(child_idx.size()); ++d)
      child_idx.at(d) = factor.at(d) * child_idx.at(d) - offset.at(d);
    return child_idx;
  }

  virtual bool invariant() const {
    bool inv =
        Common::invariant() && bool(manifold.lock()) &&
        manifold.lock()->subdiscretizations().count(name) &&
        manifold.lock()->subdiscretizations().at(name).get() == this &&
        bool(parent_discretization) &&
        parent_discretization->child_discretizations.count(name) &&
        parent_discretization->child_discretizations.at(name).lock().get() ==
            this &&
        parent_discretization->manifold.lock().get() == manifold.lock().get() &&
        bool(child_discretization) &&
        child_discretization->parent_discretizations.count(name) &&
        child_discretization->parent_discretizations.at(name).lock().get() ==
            this &&
        child_discretization.get() != parent_discretization.get() &&
        child_discretization->manifold.lock().get() == manifold.lock().get() &&
        child_discretization->configuration.get() ==
            parent_discretization->configuration.get() &&
        int(factor.size()) == manifold.lock()->dimension() &&
        int(offset.size()) == manifold.lock()->dimension();
    for (int d = 0; d < int(factor.size()); ++d)
      inv &= std::isfinite(factor.at(d)) && factor.at(d) != 0.0;
    for (int d = 0; d < int(offset.size()); ++d)
      inv &= std::isfinite(offset.at(d));
    return inv;
  }

  SubDiscretization() = delete;
  SubDiscretization(const SubDiscretization &) = delete;
  SubDiscretization(SubDiscretization &&) = delete;
  SubDiscretization &operator=(const SubDiscretization &) = delete;
  SubDiscretization &operator=(SubDiscretization &&) = delete;

  friend class Manifold;
  SubDiscretization(hidden, const string &name,
                    const shared_ptr<Manifold> &manifold,
                    const shared_ptr<Discretization> &parent_discretization,
                    const shared_ptr<Discretization> &child_discretization,
                    const vector<double> &factor, const vector<double> &offset)
      : Common(name), manifold(manifold),
        parent_discretization(parent_discretization),
        child_discretization(child_discretization), factor(factor),
        offset(offset) {}
  SubDiscretization(hidden) : Common(hidden()) {}

private:
  static shared_ptr<SubDiscretization>
  create(const string &name, const shared_ptr<Manifold> &manifold,
         const shared_ptr<Discretization> &parent_discretization,
         const shared_ptr<Discretization> &child_discretization,
         const vector<double> &factor, const vector<double> &offset) {
    auto subdiscretization = make_shared<SubDiscretization>(
        hidden(), name, manifold, parent_discretization, child_discretization,
        factor, offset);
    parent_discretization->insertChild(name, subdiscretization);
    child_discretization->insertParent(name, subdiscretization);
    return subdiscretization;
  }
  static shared_ptr<SubDiscretization>
  create(const H5::CommonFG &loc, const string &entry,
         const shared_ptr<Manifold> &manifold) {
    auto subdiscretization = make_shared<SubDiscretization>(hidden());
    subdiscretization->read(loc, entry, manifold);
    return subdiscretization;
  }
  void read(const H5::CommonFG &loc, const string &entry,
            const shared_ptr<Manifold> &manifold);

public:
  virtual ~SubDiscretization() {}

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os,
                             const SubDiscretization &subdiscretization) {
    return subdiscretization.output(os);
  }
  virtual void write(const H5::CommonFG &loc,
                     const H5::H5Location &parent) const;
};
}

#define SUBDISCRETIZATION_HPP_DONE
#endif // #ifndef SUBDISCRETIZATION_HPP
#ifndef SUBDISCRETIZATION_HPP_DONE
#error "Cyclic include depencency"
#endif
