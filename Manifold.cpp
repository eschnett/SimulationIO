#include "Manifold.hpp"

#include "Discretization.hpp"

#include "H5Helpers.hpp"

namespace SimulationIO {

Manifold::Manifold(const H5::CommonFG &loc, const string &entry,
                   Project *project)
    : project(project) {
  auto group = loc.openGroup(entry);
  string type;
  H5::readAttribute(group, "type", type);
  assert(type == "Manifold");
  H5::readAttribute(group, "name", name);
  // TODO: check link "project"
  H5::readAttribute(group, "dimension", dimension);
  H5::readGroup(group, "discretizations",
                [&](const string &name, const H5::Group &group) {
                  createDiscretization(group, name);
                },
                discretizations);
}

ostream &Manifold::output(ostream &os, int level) const {
  os << indent(level) << "Manifold \"" << name << "\": dim=" << dimension
     << "\n";
  for (const auto &d : discretizations)
    d.second->output(os, level + 1);
  return os;
}

void Manifold::write(const H5::CommonFG &loc,
                     const H5::H5Location &parent) const {
  auto group = loc.createGroup(name);
  H5::createAttribute(group, "type", "Manifold");
  H5::createAttribute(group, "name", name);
  H5::createHardLink(group, "project", parent, ".");
  // H5::createAttribute(group, "project", parent, ".");
  H5::createAttribute(group, "dimension", dimension);
  H5::createGroup(group, "discretizations", discretizations);
}

Discretization *Manifold::createDiscretization(const string &name) {
  auto discretization = new Discretization(name, this);
  checked_emplace(discretizations, discretization->name, discretization);
  assert(discretization->invariant());
  return discretization;
}

Discretization *Manifold::createDiscretization(const H5::CommonFG &loc,
                                               const string &entry) {
  auto discretization = new Discretization(loc, entry, this);
  checked_emplace(discretizations, discretization->name, discretization);
  assert(discretization->invariant());
  return discretization;
}
}
