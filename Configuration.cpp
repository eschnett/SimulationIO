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
#warning "TODO: check link project"
  H5::readGroup(group, "parametervalues", [&](const H5::Group &group,
                                              const string &name) {
    auto parname =
        H5::readGroupAttribute<string>(group, name + "/parameter", "name");
    auto parameter = project->parameters.at(parname);
    auto parametervalue = parameter->parametervalues.at(name);
    insert(parametervalue);
  });
}

ostream &Configuration::output(ostream &os, int level) const {
  os << indent(level) << "Configuration \"" << name << "\"\n";
  for (const auto &val : parametervalues)
    os << indent(level + 1) << "Parameter \"" << val.second->parameter->name
       << "\", ParameterValue \"" << val.second->name << "\"\n";
  return os;
}

void Configuration::write(const H5::CommonFG &loc,
                          const H5::H5Location &parent) const {
  auto group = loc.createGroup(name);
  H5::createAttribute(group, "type", project->enumtype, "Configuration");
  H5::createAttribute(group, "name", name);
  H5::createHardLink(group, "project", parent, ".");
  // H5::createAttribute(group, "project", parent, ".");
  auto val_group = group.createGroup("parametervalues");
  for (const auto &val : parametervalues) {
    H5::createHardLink(val_group, val.second->name, parent,
                       string("parameters/") + val.second->parameter->name +
                           "/parametervalues/" + val.second->name);
    H5::createHardLink(
        group, string("project/parameters/") + val.second->parameter->name +
                   "/parametervalues/" + val.second->name + "/configurations",
        name, group, ".");
  }
}

void Configuration::insert(ParameterValue *parametervalue) {
  checked_emplace(parametervalues, parametervalue->name, parametervalue);
}
}
