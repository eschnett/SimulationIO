#include "Parameter.hpp"

#include "H5Helpers.hpp"

namespace SimulationIO {

Parameter::Parameter(const H5::CommonFG &loc, const string &entry,
                     Project *project)
    : project(project) {
  auto group = loc.openGroup(entry);
  string type;
  H5::readAttribute(group, "type", project->enumtype, type);
  assert(type == "Parameter");
  H5::readAttribute(group, "name", name);
  // TODO: check link "project"
}

ostream &Parameter::output(ostream &os, int level) const {
  os << indent(level) << "Parameter \"" << name << "\"\n";
  return os;
}

void Parameter::write(const H5::CommonFG &loc,
                      const H5::H5Location &parent) const {
  auto group = loc.createGroup(name);
  H5::createAttribute(group, "type", project->enumtype, "Parameter");
  H5::createAttribute(group, "name", name);
  H5::createHardLink(group, "project", parent, ".");
  // H5::createAttribute(group, "project", parent, ".");
}
}
