#include "Manifold.hpp"

#include "Discretization.hpp"
#include "Field.hpp"

#include "H5Helpers.hpp"

#include <algorithm>
#include <set>

namespace SimulationIO {

using std::equal;
using std::set;

Manifold::Manifold(const H5::CommonFG &loc, const string &entry,
                   Project *project)
    : project(project) {
  auto group = loc.openGroup(entry);
  assert(H5::readAttribute<string>(group, "type", project->enumtype) ==
         "Manifold");
  H5::readAttribute(group, "name", name);
  assert(H5::readGroupAttribute<string>(group, "project", "name") ==
         project->name);
  H5::readAttribute(group, "dimension", dimension);
  H5::readGroup(group, "discretizations",
                [&](const H5::Group &group, const string &name) {
                  createDiscretization(group, name);
                });
  assert(H5::checkGroupNames(group, "fields", fields));
}

ostream &Manifold::output(ostream &os, int level) const {
  os << indent(level) << "Manifold \"" << name << "\": dim=" << dimension
     << "\n";
  for (const auto &d : discretizations)
    d.second->output(os, level + 1);
  for (const auto &f : fields)
    os << indent(level + 2) << "Field \"" << f.second->name << "\"\n";
  return os;
}

void Manifold::write(const H5::CommonFG &loc,
                     const H5::H5Location &parent) const {
  auto group = loc.createGroup(name);
  H5::createAttribute(group, "type", project->enumtype, "Manifold");
  H5::createAttribute(group, "name", name);
  H5::createHardLink(group, "project", parent, ".");
  // H5::createAttribute(group, "project", parent, ".");
  H5::createAttribute(group, "dimension", dimension);
  H5::createGroup(group, "discretizations", discretizations);
  // H5::createHardLinkGroup(group, "fields", parent, "fields", fields);
  group.createGroup("fields");
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
