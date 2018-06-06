#include "Configuration.hpp"

#include "Basis.hpp"
#include "CoordinateSystem.hpp"
#include "DiscreteField.hpp"
#include "Discretization.hpp"
#include "Field.hpp"
#include "Manifold.hpp"
#include "ParameterValue.hpp"
#include "TangentSpace.hpp"

#ifdef SIMULATIONIO_HAVE_HDF5
#include "H5Helpers.hpp"
#endif

#include <exception>
#include <sstream>

namespace SimulationIO {
using namespace std;

bool Configuration::invariant() const {
  return Common::invariant() && bool(project()) &&
         project()->configurations().count(name()) &&
         project()->configurations().at(name()).get() == this;
}

#ifdef SIMULATIONIO_HAVE_HDF5
void Configuration::read(const H5::H5Location &loc, const string &entry,
                         const shared_ptr<Project> &project) {
  m_project = project;
  auto group = loc.openGroup(entry);
  assert(H5::readAttribute<string>(group, "type", project->enumtype) ==
         "Configuration");
  H5::readAttribute(group, "name", m_name);
  assert(H5::readGroupAttribute<string>(group, "project", "name") ==
         project->name());
  H5::readGroup(group, "parametervalues",
                [&](const H5::Group &group, const string &valname) {
                  auto parname = H5::readGroupAttribute<string>(
                      group, valname + "/parameter", "name");
                  auto parameter = project->parameters().at(parname);
                  auto parametervalue =
                      parameter->parametervalues().at(valname);
                  assert(H5::readGroupAttribute<string>(
                             group, valname + "/configurations/" + name(),
                             "name") == name());
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
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
void Configuration::read(const ASDF::reader_state &rs, const YAML::Node &node,
                         const shared_ptr<Project> &project) {
  assert(node.Tag() ==
         "tag:github.com/eschnett/SimulationIO/asdf-cxx/Configuration-1.0.0");
  m_name = node["name"].Scalar();
  m_project = project;
  for (const auto &kv : node["parametervalues"]) {
    const auto &parameter = project->getParameter(rs, kv.second);
    const auto &parametervalue = parameter->getParameterValue(rs, kv.second);
    insertParameterValue(parametervalue);
  }
}
#endif

void Configuration::merge(const shared_ptr<Configuration> &configuration) {
  assert(project()->name() == configuration->project()->name());
  for (const auto &iter : configuration->parametervalues()) {
    // TODO: Check for identical parameters, instead of parametervalue names
    const auto &parametervalue = iter.second;
    if (!m_parametervalues.count(parametervalue->name()))
      insertParameterValue(project()
                               ->parameters()
                               .at(parametervalue->parameter()->name())
                               ->parametervalues()
                               .at(parametervalue->name()));
  }
}

ostream &Configuration::output(ostream &os, int level) const {
  os << indent(level) << "Configuration " << quote(name()) << "\n";
  for (const auto &val : parametervalues())
    os << indent(level + 1) << "Parameter "
       << quote(val.second->parameter()->name()) << " ParameterValue "
       << quote(val.second->name()) << "\n";
  for (const auto &b : bases())
    os << indent(level + 1) << "Basis " << quote(b.second.lock()->name())
       << "\n";
  for (const auto &cs : coordinatesystems())
    os << indent(level + 1) << "CoordinateSystem "
       << quote(cs.second.lock()->name()) << "\n";
  for (const auto &df : discretefields())
    os << indent(level + 1) << "DiscreteField "
       << quote(df.second.lock()->name()) << "\n";
  for (const auto &d : discretizations())
    os << indent(level + 1) << "Discretization "
       << quote(d.second.lock()->name()) << "\n";
  for (const auto &f : fields())
    os << indent(level + 1) << "Field " << quote(f.second.lock()->name())
       << "\n";
  for (const auto &m : manifolds())
    os << indent(level + 1) << "Manifold " << quote(m.second.lock()->name())
       << "\n";
  for (const auto &s : tangentspaces())
    os << indent(level + 1) << "TangentSpace " << quote(s.second.lock()->name())
       << "\n";
  return os;
}

#ifdef SIMULATIONIO_HAVE_HDF5
void Configuration::write(const H5::H5Location &loc,
                          const H5::H5Location &parent) const {
  assert(invariant());
  auto group = loc.createGroup(name());
  H5::createAttribute(group, "type", project()->enumtype, "Configuration");
  H5::createAttribute(group, "name", name());
  // H5::createHardLink(group, "project", parent, ".");
  H5::createHardLink(group, "..", parent, ".");
  H5::createSoftLink(group, "project", "..");
  auto val_group = group.createGroup("parametervalues");
  for (const auto &val : parametervalues()) {
    H5::createHardLink(val_group, val.second->name(), parent,
                       "parameters/" + val.second->parameter()->name() +
                           "/parametervalues/" + val.second->name());
    H5::createHardLink(group,
                       "project/parameters/" + val.second->parameter()->name() +
                           "/parametervalues/" + val.second->name() +
                           "/configurations",
                       name(), group, ".");
  }
  group.createGroup("bases");
  group.createGroup("coordinatesystems");
  group.createGroup("discretefields");
  group.createGroup("discretizations");
  group.createGroup("fields");
  group.createGroup("manifolds");
  group.createGroup("tangentspaces");
}
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
vector<string> Configuration::yaml_path() const {
  return concat(project()->yaml_path(), {"configurations", name()});
}

ASDF::writer &Configuration::write(ASDF::writer &w) const {
  auto aw = asdf_writer(w);
  aw.alias_group("parametervalues", parametervalues());
  return w;
}
#endif

void Configuration::insertParameterValue(
    const shared_ptr<ParameterValue> &parametervalue) {
  assert(parametervalue->parameter()->project().get() == project().get());
  for (const auto &val : parametervalues()) {
    if (val.second->parameter().get() == parametervalue->parameter().get()) {
      ostringstream buf;
      buf << "Cannot merge Configurations \"" << name()
          << "\" with conflicting Parameter settings:\n"
          << indent(1) << "current:\n";
      parametervalue->output(buf, 2);
      buf << indent(1) << "new:\n";
      val.second->output(buf, 2);
      throw range_error(buf.str());
    }
  }
  checked_emplace(m_parametervalues, parametervalue->name(), parametervalue,
                  "Configuration", "parametervalues");
  parametervalue->insert(shared_from_this());
}

} // namespace SimulationIO
