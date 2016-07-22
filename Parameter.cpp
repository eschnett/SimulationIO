#include "Parameter.hpp"

#include "ParameterValue.hpp"

#include "H5Helpers.hpp"

namespace SimulationIO {

void Parameter::read(const H5::CommonFG &loc, const string &entry,
                     const shared_ptr<Project> &project) {
  m_project = project;
  auto group = loc.openGroup(entry);
  assert(H5::readAttribute<string>(group, "type", project->enumtype) ==
         "Parameter");
  H5::readAttribute(group, "name", m_name);
  assert(H5::readGroupAttribute<string>(group, "project", "name") ==
         project->name());
  H5::readGroup(group, "parametervalues",
                [&](const H5::Group &group, const string &name) {
                  readParameterValue(group, name);
                });
}

void Parameter::merge(const shared_ptr<Parameter> &parameter) {
  assert(project()->name() == parameter->project()->name());
  for (const auto &iter : parameter->parametervalues()) {
    const auto &parametervalue = iter.second;
    if (!m_parametervalues.count(parametervalue->name()))
      createParameterValue(parametervalue->name());
    m_parametervalues.at(parametervalue->name())->merge(parametervalue);
  }
}

ostream &Parameter::output(ostream &os, int level) const {
  os << indent(level) << "Parameter " << quote(name()) << "\n";
  for (const auto &val : parametervalues())
    val.second->output(os, level + 1);
  return os;
}

void Parameter::write(const H5::CommonFG &loc,
                      const H5::H5Location &parent) const {
  assert(invariant());
  auto group = loc.createGroup(name());
  H5::createAttribute(group, "type", project()->enumtype, "Parameter");
  H5::createAttribute(group, "name", name());
  // H5::createHardLink(group, "project", parent, ".");
  H5::createHardLink(group, "..", parent, ".");
  H5::createSoftLink(group, "project", "..");
  H5::createGroup(group, "parametervalues", parametervalues());
}

shared_ptr<ParameterValue> Parameter::createParameterValue(const string &name) {
  auto parametervalue = ParameterValue::create(name, shared_from_this());
  checked_emplace(m_parametervalues, parametervalue->name(), parametervalue,
                  "Parameter", "parametervalues");
  assert(parametervalue->invariant());
  return parametervalue;
}

shared_ptr<ParameterValue>
Parameter::readParameterValue(const H5::CommonFG &loc, const string &entry) {
  auto parametervalue = ParameterValue::create(loc, entry, shared_from_this());
  checked_emplace(m_parametervalues, parametervalue->name(), parametervalue,
                  "Parameter", "parametervalues");
  assert(parametervalue->invariant());
  return parametervalue;
}
}
