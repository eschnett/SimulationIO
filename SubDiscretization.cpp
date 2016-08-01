#include "SubDiscretization.hpp"

#include "Discretization.hpp"

#include "H5Helpers.hpp"

#include <algorithm>

namespace SimulationIO {

void SubDiscretization::read(const H5::CommonFG &loc, const string &entry,
                             const shared_ptr<Manifold> &manifold) {
  m_manifold = manifold;
  auto group = loc.openGroup(entry);
  assert(
      H5::readAttribute<string>(group, "type", manifold->project()->enumtype) ==
      "SubDiscretization");
  H5::readAttribute(group, "name", m_name);
  assert(H5::readGroupAttribute<string>(group, "manifold", "name") ==
         manifold->name());
  m_parent_discretization = manifold->discretizations().at(
      H5::readGroupAttribute<string>(group, "parent_discretization", "name"));
  assert(H5::readGroupAttribute<string>(
             group, "parent_discretization/child_discretizations/" + name(),
             "name") == name());
  m_child_discretization = manifold->discretizations().at(
      H5::readGroupAttribute<string>(group, "child_discretization", "name"));
  assert(H5::readGroupAttribute<string>(
             group, "child_discretization/parent_discretizations/" + name(),
             "name") == name());
  H5::readAttribute(group, "factor", m_factor);
  std::reverse(m_factor.begin(), m_factor.end());
  H5::readAttribute(group, "offset", m_offset);
  std::reverse(m_offset.begin(), m_offset.end());
  m_parent_discretization->insertChild(name(), shared_from_this());
  m_child_discretization->insertParent(name(), shared_from_this());
}

void SubDiscretization::merge(
    const shared_ptr<SubDiscretization> &subdiscretization) {
  assert(manifold()->name() == subdiscretization->manifold()->name());
  assert(m_parent_discretization->name() ==
         subdiscretization->parent_discretization()->name());
  assert(m_child_discretization->name() ==
         subdiscretization->child_discretization()->name());
  assert(m_factor == subdiscretization->factor());
  assert(m_offset == subdiscretization->offset());
}

ostream &SubDiscretization::output(ostream &os, int level) const {
  os << indent(level) << "SubDiscretization " << quote(name()) << ": Manifold "
     << quote(manifold()->name()) << " parent Discretization "
     << quote(parent_discretization()->name()) << " child Discretization "
     << quote(child_discretization()->name()) << " factor=" << factor()
     << " offset=" << offset() << "\n";
  return os;
}

void SubDiscretization::write(const H5::CommonFG &loc,
                              const H5::H5Location &parent) const {
  assert(invariant());
  auto group = loc.createGroup(name());
  H5::createAttribute(group, "type", manifold()->project()->enumtype,
                      "SubDiscretization");
  H5::createAttribute(group, "name", name());
  // H5::createHardLink(group, "manifold", parent, ".");
  H5::createHardLink(group, "..", parent, ".");
  H5::createSoftLink(group, "manifold", "..");
  // H5::createHardLink(group, "parent_discretization", parent,
  //                    "discretizations/" +
  //                    parent_discretization->name());
  H5::createSoftLink(group, "parent_discretization",
                     "../discretizations/" + parent_discretization()->name());
  H5::createHardLink(group, "manifold/discretizations/" +
                                parent_discretization()->name() +
                                "/child_discretizations",
                     name(), group, ".");
  // H5::createHardLink(group, "child_discretization", parent,
  //                    "discretizations/" +
  //                    child_discretization->name());
  H5::createSoftLink(group, "child_discretization",
                     "../discretizations/" + child_discretization()->name());
  H5::createHardLink(group, "manifold/discretizations/" +
                                child_discretization()->name() +
                                "/parent_discretizations",
                     name(), group, ".");
  auto tmp_factor = factor();
  std::reverse(tmp_factor.begin(), tmp_factor.end());
  H5::createAttribute(group, "factor", tmp_factor);
  auto tmp_offset = offset();
  std::reverse(tmp_offset.begin(), tmp_offset.end());
  H5::createAttribute(group, "offset", tmp_offset);
}
}
