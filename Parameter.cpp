#include "Parameter.hpp"

#include "ParameterValue.hpp"

#include "H5Helpers.hpp"

namespace SimulationIO {

void Parameter::read(const H5::CommonFG &loc, const string &entry,
                     const shared_ptr<Project> &project) {
  this->project = project;
  auto group = loc.openGroup(entry);
  assert(H5::readAttribute<string>(group, "type", project->enumtype) ==
         "Parameter");
  H5::readAttribute(group, "name", name);
  assert(H5::readGroupAttribute<string>(group, "project", "name") ==
         project->name);
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
  H5::createGroup(group, "parametervalues", parametervalues);
}

shared_ptr<ParameterValue> Parameter::createParameterValue(const string &name) {
  auto parametervalue = ParameterValue::create(name, shared_from_this());
  checked_emplace(parametervalues, parametervalue->name, parametervalue);
  assert(parametervalue->invariant());
  return parametervalue;
}

shared_ptr<ParameterValue>
Parameter::createParameterValue(const H5::CommonFG &loc, const string &entry) {
  auto parametervalue = ParameterValue::create(loc, entry, shared_from_this());
  checked_emplace(parametervalues, parametervalue->name, parametervalue);
  assert(parametervalue->invariant());
  return parametervalue;
}
}
