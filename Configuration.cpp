#include "Configuration.hpp"

#include "ParameterValue.hpp"

#include "H5Helpers.hpp"

namespace SimulationIO {

void Configuration::read(const H5::CommonFG &loc, const string &entry,
                         const shared_ptr<Project> &project) {
  this->project = project;
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
    insertParameterValue(parametervalue);
  });
}

ostream &Configuration::output(ostream &os, int level) const {
  os << indent(level) << "Configuration \"" << name << "\"\n";
  for (const auto &val : parametervalues)
    os << indent(level + 1) << "Parameter \""
       << val.second->parameter.lock()->name << "\" ParameterValue \""
       << val.second->name << "\"\n";
  return os;
}

void Configuration::write(const H5::CommonFG &loc,
                          const H5::H5Location &parent) const {
  auto group = loc.createGroup(name);
  H5::createAttribute(group, "type", project.lock()->enumtype, "Configuration");
  H5::createAttribute(group, "name", name);
  H5::createHardLink(group, "project", parent, ".");
  auto val_group = group.createGroup("parametervalues");
  for (const auto &val : parametervalues) {
    H5::createHardLink(val_group, val.second->name, parent,
                       string("parameters/") +
                           val.second->parameter.lock()->name +
                           "/parametervalues/" + val.second->name);
    H5::createHardLink(group, string("project/parameters/") +
                                  val.second->parameter.lock()->name +
                                  "/parametervalues/" + val.second->name +
                                  "/configurations",
                       name, group, ".");
  }
}

void Configuration::insertParameterValue(
    const shared_ptr<ParameterValue> &parametervalue) {
  for (const auto &val : parametervalues)
    assert(val.second->parameter.lock().get() !=
           parametervalue->parameter.lock().get());
  checked_emplace(parametervalues, parametervalue->name, parametervalue);
  parametervalue->insert(shared_from_this());
}
}
