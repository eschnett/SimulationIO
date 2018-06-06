#include "Parameter.hpp"

#include "ParameterValue.hpp"

#ifdef SIMULATIONIO_HAVE_HDF5
#include "H5Helpers.hpp"
#endif

namespace SimulationIO {

bool Parameter::invariant() const {
  return Common::invariant() && bool(project()) &&
         project()->parameters().count(name()) &&
         project()->parameters().at(name()).get() == this;
}

#ifdef SIMULATIONIO_HAVE_HDF5
void Parameter::read(const H5::H5Location &loc, const string &entry,
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
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
void Parameter::read(const ASDF::reader_state &rs, const YAML::Node &node,
                     const shared_ptr<Project> &project) {
  assert(node.Tag() ==
         "tag:github.com/eschnett/SimulationIO/asdf-cxx/Parameter-1.0.0");
  m_name = node["name"].Scalar();
  m_project = project;
  for (const auto &kv : node["parametervalues"])
    readParameterValue(rs, kv.second);
}
#endif

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

#ifdef SIMULATIONIO_HAVE_HDF5
void Parameter::write(const H5::H5Location &loc,
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
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
vector<string> Parameter::yaml_path() const {
  return concat(project()->yaml_path(), {"parameters", name()});
}

ASDF::writer &Parameter::write(ASDF::writer &w) const {
  auto aw = asdf_writer(w);
  aw.group("parametervalues", parametervalues());
  return w;
}
#endif

shared_ptr<ParameterValue> Parameter::createParameterValue(const string &name) {
  auto parametervalue = ParameterValue::create(name, shared_from_this());
  checked_emplace(m_parametervalues, parametervalue->name(), parametervalue,
                  "Parameter", "parametervalues");
  assert(parametervalue->invariant());
  return parametervalue;
}

shared_ptr<ParameterValue> Parameter::getParameterValue(const string &name) {
  auto loc = m_parametervalues.find(name);
  if (loc != m_parametervalues.end()) {
    const auto &parametervalue = loc->second;
    return parametervalue;
  }
  return createParameterValue(name);
}

shared_ptr<ParameterValue>
Parameter::copyParameterValue(const shared_ptr<ParameterValue> &parametervalue,
                              bool copy_children) {
  auto parametervalue2 = getParameterValue(parametervalue->name());
  switch (parametervalue->getValueType()) {
  case ParameterValue::type_empty:
    parametervalue2->setValue();
    break;
  case ParameterValue::type_int: {
    auto value = parametervalue->getValueInt();
    parametervalue2->setValue(value);
    break;
  }
  case ParameterValue::type_double: {
    auto value = parametervalue->getValueDouble();
    parametervalue2->setValue(value);
    break;
  }
  case ParameterValue::type_string: {
    auto value = parametervalue->getValueString();
    parametervalue2->setValue(value);
    break;
  }
  default:
    assert(0);
  }
  return parametervalue2;
}

#ifdef SIMULATIONIO_HAVE_HDF5
shared_ptr<ParameterValue>
Parameter::readParameterValue(const H5::H5Location &loc, const string &entry) {
  auto parametervalue = ParameterValue::create(loc, entry, shared_from_this());
  checked_emplace(m_parametervalues, parametervalue->name(), parametervalue,
                  "Parameter", "parametervalues");
  assert(parametervalue->invariant());
  return parametervalue;
}
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
shared_ptr<ParameterValue>
Parameter::readParameterValue(const ASDF::reader_state &rs,
                              const YAML::Node &node) {
  auto parametervalue = ParameterValue::create(rs, node, shared_from_this());
  checked_emplace(m_parametervalues, parametervalue->name(), parametervalue,
                  "Parameter", "parametervalues");
  assert(parametervalue->invariant());
  return parametervalue;
}

shared_ptr<ParameterValue>
Parameter::getParameterValue(const ASDF::reader_state &rs,
                             const YAML::Node &node) {
  auto ref = ASDF::reference(rs, node);
  auto doc_path = ref.get_split_target();
  const auto &doc = doc_path.first;
  const auto &path = doc_path.second;
  assert(doc.empty());
  assert(path.at(0) == project()->name());
  assert(path.at(1) == "parameters");
  assert(path.at(2) == name());
  assert(path.at(3) == "parametervalues");
  const auto &parametervalue_name = path.at(4);
  return parametervalues().at(parametervalue_name);
}
#endif

} // namespace SimulationIO
