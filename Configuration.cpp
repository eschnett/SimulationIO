#include "Configuration.hpp"

#include "ParameterValue.hpp"

#include "H5Helpers.hpp"

namespace SimulationIO {

Configuration::Configuration(const H5::CommonFG &loc, const string &entry,
                             Project *project)
    : project(project) {
  auto group = loc.openGroup(entry);
  string type;
  H5::readAttribute(group, "type", project->enumtype, type);
  assert(type == "Configuration");
  H5::readAttribute(group, "name", name);
  // TODO: check link "project"
  H5::readGroup(group, "parametervalues",
                [&](const string &name, const H5::Group &group) {
                  createParameterValue(group, name);
                },
                parametervalues);
}

ostream &Configuration::output(ostream &os, int level) const {
  os << indent(level) << "Configuration \"" << name << "\"\n";
  // TODO for (const auto &b : parameters)
  // TODO   b.second->output(os, level + 1);
  return os;
}

void Configuration::write(const H5::CommonFG &loc,
                          const H5::H5Location &parent) const {
  auto group = loc.createGroup(name);
  H5::createAttribute(group, "type", project->enumtype, "Configuration");
  H5::createAttribute(group, "name", name);
  H5::createHardLink(group, "project", parent, ".");
  // H5::createAttribute(group, "project", parent, ".");
  H5::createGroup(group, "parametervalues", parametervalues);
}

ParameterValue *Configuration::createParameterValue(const string &name,
                                                    Parameter *parameter) {
  auto parametervalue = new ParameterValue(name, this, parameter);
  checked_emplace(parametervalues, parametervalue->name, parametervalue);
  assert(parametervalue->invariant());
  return parametervalue;
}

ParameterValue *Configuration::createParameterValue(const H5::CommonFG &loc,
                                                    const string &entry) {
  auto parametervalue = new ParameterValue(loc, entry, this);
  checked_emplace(parametervalues, parametervalue->name, parametervalue);
  assert(parametervalue->invariant());
  return parametervalue;
}
}
