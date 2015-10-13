#include "Field.hpp"

#include "H5Helpers.hpp"

namespace SimulationIO {

Field::Field(const H5::CommonFG &loc, const string &entry, Project *project)
    : project(project) {
  auto group = loc.openGroup(entry);
  string type;
  H5::readAttribute(group, "type", type);
  assert(type == "Field");
  H5::readAttribute(group, "name", name);
  // TODO: check link "project"
  // TODO: Read and interpret objects (shallowly) instead of naively only
  // looking at their names
  {
    H5::Group obj;
    H5::readAttribute(group, "manifold", obj);
    string name;
    auto attr = H5::readAttribute(obj, "name", name);
    manifold = project->manifolds.at(name);
  }
  {
    H5::Group obj;
    H5::readAttribute(group, "tangentspace", obj);
    string name;
    auto attr = H5::readAttribute(obj, "name", name);
    tangentspace = project->tangentspaces.at(name);
  }
  {
    H5::Group obj;
    H5::readAttribute(group, "tensortype", obj);
    string name;
    auto attr = H5::readAttribute(obj, "name", name);
    tensortype = project->tensortypes.at(name);
  }
#warning "TODO"
  // H5::readGroup(group, "discretefields",
  //                [&](const string &name, const H5::Group &group) {
  //                  createDiscreteField(name, group);
  //                },
  //                discretefields);
  manifold->insert(name, this);
  tangentspace->insert(name, this);
  // tensortypes->insert(this);
}

ostream &Field::output(ostream &os, int level) const {
  os << indent(level) << "Field \"" << name << "\": manifold=\""
     << manifold->name << "\" tangentspace=\"" << tangentspace->name
     << "\" tensortype=\"" << tensortype->name << "\"\n";
#warning "TODO"
  // for (const auto &df : discretefields)
  //   df.second->output(os, level + 1);
  return os;
}

void Field::write(const H5::CommonFG &loc, const H5::H5Location &parent) const {
  auto group = loc.createGroup(name);
  H5::createAttribute(group, "type", "Field");
  H5::createAttribute(group, "name", name);
  H5::createAttribute(group, "project", parent, ".");
  H5::createAttribute(group, "manifold", parent,
                      string("manifolds/") + manifold->name);
  H5::createAttribute(group, "tangentspace", parent,
                      string("tangentspaces/") + tangentspace->name);
  H5::createAttribute(group, "tensortype", parent,
                      string("tensortypes/") + tensortype->name);
#warning "TODO"
  // H5::createGroup(group, "discretefields", discretefields);
}
}
