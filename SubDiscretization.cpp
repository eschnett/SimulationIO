#include "SubDiscretization.hpp"

#include "Discretization.hpp"

#include "H5Helpers.hpp"

#include <algorithm>

namespace SimulationIO {

void SubDiscretization::read(const H5::CommonFG &loc, const string &entry,
                             const shared_ptr<Manifold> &manifold) {
  this->manifold = manifold;
  auto group = loc.openGroup(entry);
  assert(H5::readAttribute<string>(group, "type",
                                   manifold->project.lock()->enumtype) ==
         "SubDiscretization");
  H5::readAttribute(group, "name", name);
  assert(H5::readGroupAttribute<string>(group, "manifold", "name") ==
         manifold->name);
  parent_discretization = manifold->discretizations.at(
      H5::readGroupAttribute<string>(group, "parent_discretization", "name"));
  assert(H5::readGroupAttribute<string>(
             group,
             string("parent_discretization/child_discretizations/") + name,
             "name") == name);
  child_discretization = manifold->discretizations.at(
      H5::readGroupAttribute<string>(group, "child_discretization", "name"));
  assert(H5::readGroupAttribute<string>(
             group,
             string("child_discretization/parent_discretizations/") + name,
             "name") == name);
  H5::readAttribute(group, "factor", factor);
  std::reverse(factor.begin(), factor.end());
  H5::readAttribute(group, "offset", offset);
  std::reverse(offset.begin(), offset.end());
  parent_discretization->insertChild(name, shared_from_this());
  child_discretization->insertParent(name, shared_from_this());
}

ostream &SubDiscretization::output(ostream &os, int level) const {
  os << indent(level) << "SubDiscretization " << quote(name) << ": Manifold "
     << quote(manifold.lock()->name) << " parent Discretization "
     << quote(parent_discretization->name) << " child Discretization "
     << quote(child_discretization->name) << " factor=" << factor
     << " offset=" << offset << "\n";
  return os;
}

void SubDiscretization::write(const H5::CommonFG &loc,
                              const H5::H5Location &parent) const {
  assert(invariant());
  auto group = loc.createGroup(name);
  H5::createAttribute(group, "type", manifold.lock()->project.lock()->enumtype,
                      "SubDiscretization");
  H5::createAttribute(group, "name", name);
  // H5::createHardLink(group, "manifold", parent, ".");
  H5::createHardLink(group, "..", parent, ".");
  H5::createSoftLink(group, "manifold", "..");
  // H5::createHardLink(group, "parent_discretization", parent,
  //                    string("discretizations/") +
  //                    parent_discretization->name);
  H5::createSoftLink(group, "parent_discretization",
                     string("../discretizations/") +
                         parent_discretization->name);
  H5::createHardLink(group,
                     string("manifold/discretizations/") +
                         parent_discretization->name + "/child_discretizations",
                     name, group, ".");
  // H5::createHardLink(group, "child_discretization", parent,
  //                    string("discretizations/") +
  //                    child_discretization->name);
  H5::createSoftLink(group, "child_discretization",
                     string("../discretizations/") +
                         child_discretization->name);
  H5::createHardLink(group,
                     string("manifold/discretizations/") +
                         child_discretization->name + "/parent_discretizations",
                     name, group, ".");
  auto tmp_factor = factor;
  std::reverse(tmp_factor.begin(), tmp_factor.end());
  H5::createAttribute(group, "factor", tmp_factor);
  auto tmp_offset = offset;
  std::reverse(tmp_offset.begin(), tmp_offset.end());
  H5::createAttribute(group, "offset", tmp_offset);
}
}
