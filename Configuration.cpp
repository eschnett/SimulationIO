#include "Configuration.hpp"

#include "ParameterValue.hpp"

#include "H5Helpers.hpp"

namespace SimulationIO {

Configuration::Configuration(const H5::CommonFG &loc, const string &entry,
                             Project *project)
    : project(project) {
  auto group = loc.openGroup(entry);
  assert(H5::readAttribute<string>(group, "type", project->enumtype) ==
         "Configuration");
  H5::readAttribute(group, "name", name);
  assert(H5::readGroupAttribute<string>(group, "project", "name") ==
         project->name);
  H5::readGroup(group, "parametervalues", [&](const H5::Group &group,
                                              const string &valname) {
    auto parname =
        H5::readGroupAttribute<string>(group, valname + "/parameter", "name");
    auto parameter = project->parameters.at(parname);
    auto parametervalue = parameter->parametervalues.at(valname);
    assert(H5::readGroupAttribute<string>(
               group, valname + string("/configurations/") + name, "name") ==
           name);
    insert(parametervalue);
  });
}

ostream &Configuration::output(ostream &os, int level) const {
  os << indent(level) << "Configuration \"" << name << "\"\n";
  for (const auto &val : parametervalues)
    os << indent(level + 1) << "Parameter \"" << val.second->parameter->name
       << "\" ParameterValue \"" << val.second->name << "\"\n";
  return os;
}

void Configuration::write(const H5::CommonFG &loc,
                          const H5::H5Location &parent) const {
  auto group = loc.createGroup(name);
  H5::createAttribute(group, "type", project->enumtype, "Configuration");
  H5::createAttribute(group, "name", name);
  H5::createHardLink(group, "project", parent, ".");
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
  for (const auto &val : parametervalues)
    assert(val.second->parameter != parametervalue->parameter);
  checked_emplace(parametervalues, parametervalue->name, parametervalue);
}
}