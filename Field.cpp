#include "Field.hpp"

#include "DiscreteField.hpp"

#include "H5Helpers.hpp"

namespace SimulationIO {

Field::Field(const H5::CommonFG &loc, const string &entry, Project *project)
    : project(project) {
  auto group = loc.openGroup(entry);
  assert(H5::readAttribute<string>(group, "type", project->enumtype) ==
         "Field");
  H5::readAttribute(group, "name", name);
  assert(H5::readGroupAttribute<string>(group, "project", "name") ==
         project->name);
  // TODO: Read and interpret objects (shallowly) instead of naively only
  // looking at their names
  manifold = project->manifolds.at(
      H5::readGroupAttribute<string>(group, "manifold", "name"));
  assert(H5::readGroupAttribute<string>(
             group, string("manifold/fields/") + name, "name") == name);
  tangentspace = project->tangentspaces.at(
      H5::readGroupAttribute<string>(group, "tangentspace", "name"));
  assert(H5::readGroupAttribute<string>(
             group, string("tangentspace/fields/") + name, "name") == name);
  tensortype = project->tensortypes.at(
      H5::readGroupAttribute<string>(group, "tensortype", "name"));
  H5::readGroup(group, "discretefields",
                [&](const H5::Group &group, const string &name) {
                  createDiscreteField(group, name);
                });
  manifold->insert(name, this);
  tangentspace->insert(name, this);
  tensortype->noinsert(this);
}

ostream &Field::output(ostream &os, int level) const {
  os << indent(level) << "Field \"" << name << "\": Manifold \""
     << manifold->name << "\" TangentSpace \"" << tangentspace->name
     << "\" TensorType \"" << tensortype->name << "\"\n";
  for (const auto &df : discretefields)
    df.second->output(os, level + 1);
  return os;
}

void Field::write(const H5::CommonFG &loc, const H5::H5Location &parent) const {
  auto group = loc.createGroup(name);
  H5::createAttribute(group, "type", project->enumtype, "Field");
  H5::createAttribute(group, "name", name);
  H5::createHardLink(group, "project", parent, ".");
  H5::createHardLink(group, "manifold", parent,
                     string("manifolds/") + manifold->name);
  H5::createHardLink(group,
                     string("project/manifolds/") + manifold->name + "/fields",
                     name, group, ".");
  H5::createHardLink(group, "tangentspace", parent,
                     string("tangentspaces/") + tangentspace->name);
  H5::createHardLink(group, string("project/tangentspaces/") +
                                tangentspace->name + "/fields",
                     name, group, ".");
  H5::createHardLink(group, "tensortype", parent,
                     string("tensortypes/") + tensortype->name);
  H5::createGroup(group, "discretefields", discretefields);
}

DiscreteField *Field::createDiscreteField(const string &name,
                                          Discretization *discretization,
                                          Basis *basis) {
  auto discretefield = new DiscreteField(name, this, discretization, basis);
  checked_emplace(discretefields, discretefield->name, discretefield);
  assert(discretefield->invariant());
  return discretefield;
}
DiscreteField *Field::createDiscreteField(const H5::CommonFG &loc,
                                          const string &entry) {
  auto discretefield = new DiscreteField(loc, entry, this);
  checked_emplace(discretefields, discretefield->name, discretefield);
  assert(discretefield->invariant());
  return discretefield;
}
}
