#include "Configuration.hpp"

#include "Basis.hpp"
#include "CoordinateSystem.hpp"
#include "DiscreteField.hpp"
#include "Discretization.hpp"
#include "Field.hpp"
#include "Manifold.hpp"
#include "ParameterValue.hpp"
#include "TangentSpace.hpp"

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
  // Cannot check "bases", "coordinatesystems", "discretefields",
  // "discretizations", "fields", "manifolds", "tangentspaces" since they have
  // not been read yet
  // assert(H5::checkGroupNames(group, "bases", manifolds));
  // assert(H5::checkGroupNames(group, "coordinatesystems", manifolds));
  // assert(H5::checkGroupNames(group, "discretefields", manifolds));
  // assert(H5::checkGroupNames(group, "discretizations", manifolds));
  // assert(H5::checkGroupNames(group, "fields", manifolds));
  // assert(H5::checkGroupNames(group, "manifolds", manifolds));
  // assert(H5::checkGroupNames(group, "tangentspaces", manifolds));
}

ostream &Configuration::output(ostream &os, int level) const {
  os << indent(level) << "Configuration " << quote(name) << "\n";
  for (const auto &val : parametervalues)
    os << indent(level + 1) << "Parameter "
       << quote(val.second->parameter.lock()->name) << " ParameterValue "
       << quote(val.second->name) << "\n";
  for (const auto &b : bases)
    os << indent(level + 1) << "Basis " << quote(b.second.lock()->name) << "\n";
  for (const auto &cs : coordinatesystems)
    os << indent(level + 1) << "CoordinateSystem "
       << quote(cs.second.lock()->name) << "\n";
  for (const auto &df : discretefields)
    os << indent(level + 1) << "DiscreteField " << quote(df.second.lock()->name)
       << "\n";
  for (const auto &d : discretizations)
    os << indent(level + 1) << "Discretization " << quote(d.second.lock()->name)
       << "\n";
  for (const auto &f : fields)
    os << indent(level + 1) << "Field " << quote(f.second.lock()->name) << "\n";
  for (const auto &m : manifolds)
    os << indent(level + 1) << "Manifold " << quote(m.second.lock()->name)
       << "\n";
  for (const auto &s : tangentspaces)
    os << indent(level + 1) << "TangentSpace " << quote(s.second.lock()->name)
       << "\n";
  return os;
}

void Configuration::write(const H5::CommonFG &loc,
                          const H5::H5Location &parent) const {
  assert(invariant());
  auto group = loc.createGroup(name);
  H5::createAttribute(group, "type", project.lock()->enumtype, "Configuration");
  H5::createAttribute(group, "name", name);
  // H5::createHardLink(group, "project", parent, ".");
  H5::createHardLink(group, "..", parent, ".");
  H5::createSoftLink(group, "project", "..");
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
  group.createGroup("bases");
  group.createGroup("coordinatesystems");
  group.createGroup("discretefields");
  group.createGroup("discretizations");
  group.createGroup("fields");
  group.createGroup("manifolds");
  group.createGroup("tangentspaces");
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
