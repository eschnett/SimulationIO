#include "Parameter.hpp"

#include "ParameterValue.hpp"

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
#warning "TODO: check link project"
  H5::readGroup(group, "parametervalues",
                [&](const H5::Group &group, const string &name) {
                  createParameterValue(group, name);
                });
}

ostream &Parameter::output(ostream &os, int level) const {
  os << indent(level) << "Parameter \"" << name << "\"\n";
  for (const auto &val : parametervalues)
    val.second->output(os, level + 1);
  return os;
}

void Parameter::write(const H5::CommonFG &loc,
                      const H5::H5Location &parent) const {
  auto group = loc.createGroup(name);
  H5::createAttribute(group, "type", project->enumtype, "Parameter");
  H5::createAttribute(group, "name", name);
  H5::createHardLink(group, "project", parent, ".");
  // H5::createAttribute(group, "project", parent, ".");
  H5::createGroup(group, "parametervalues", parametervalues);
}

ParameterValue *Parameter::createParameterValue(const string &name) {
  auto parametervalue = new ParameterValue(name, this);
  checked_emplace(parametervalues, parametervalue->name, parametervalue);
  assert(parametervalue->invariant());
  return parametervalue;
}

ParameterValue *Parameter::createParameterValue(const H5::CommonFG &loc,
                                                const string &entry) {
  auto parametervalue = new ParameterValue(loc, entry, this);
  checked_emplace(parametervalues, parametervalue->name, parametervalue);
  assert(parametervalue->invariant());
  return parametervalue;
}
}
