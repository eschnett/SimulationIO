#include "ParameterValue.hpp"

#include "Configuration.hpp"
#include "Parameter.hpp"

#ifdef SIMULATIONIO_HAVE_HDF5
#include "H5Helpers.hpp"
#endif

#include <exception>
#include <sstream>

namespace SimulationIO {
using namespace std;

bool ParameterValue::invariant() const {
  return Common::invariant() && bool(parameter()) &&
         parameter()->parametervalues().count(name()) &&
         parameter()->parametervalues().at(name()).get() == this &&
         value_type >= type_empty && value_type <= type_string;
}

#ifdef SIMULATIONIO_HAVE_HDF5
void ParameterValue::read(const H5::H5Location &loc, const string &entry,
                          const shared_ptr<Parameter> &parameter) {
  m_parameter = parameter;
  value_type = type_empty;
  auto group = loc.openGroup(entry);
  assert(H5::readAttribute<string>(group, "type",
                                   parameter->project()->enumtype) ==
         "ParameterValue");
  H5::readAttribute(group, "name", m_name);
  assert(H5::readGroupAttribute<string>(group, "parameter", "name") ==
         parameter->name());
  // TODO: Read and interpret objects (shallowly) instead of naively only
  // looking at their names
  if (group.attrExists("data")) {
    auto attr = group.openAttribute("data");
    auto type = attr.getDataType();
    auto cls = type.getClass();
    switch (cls) {
    case H5T_INTEGER:
      H5::readAttribute(group, "data", value_int);
      value_type = type_int;
      break;
    case H5T_FLOAT:
      H5::readAttribute(group, "data", value_double);
      value_type = type_double;
      break;
    case H5T_STRING:
      H5::readAttribute(group, "data", value_string);
      value_type = type_string;
      break;
    default:
      assert(0);
    }
  } else {
    H5E_BEGIN_TRY {
      try {
        H5::readDataSet(group, "data", value_string);
        value_type = type_string;
      } catch (const H5::FileIException &ex) {
        // do nothing
      } catch (const H5::GroupIException &ex) {
        // do nothing
      }
    }
    H5E_END_TRY;
  }
  // Cannot check "configurations" since configurations have not been read yet
  // assert(H5::checkGroupNames(group, "configurations", configurations));
}
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
void ParameterValue::read(const shared_ptr<ASDF::reader_state> &rs,
                          const YAML::Node &node,
                          const shared_ptr<Parameter> &parameter) {
  assert(node.Tag() ==
         "tag:github.com/eschnett/SimulationIO/asdf-cxx/ParameterValue-1.0.0");
  m_name = node["name"].Scalar();
  m_parameter = parameter;
  // TODO: Read and interpret objects (shallowly) instead of naively only
  // looking at their names
  const auto &data = node["data"];
  if (data.IsDefined()) {
    const auto &type = node["type"].Scalar();
    if (type == "int") {
      value_type = type_int;
      value_int = data.as<long long>();
    } else if (type == "float") {
      value_type = type_double;
      value_double = data.as<double>();
    } else if (type == "string") {
      value_type = type_string;
      value_string = data.as<string>();
    } else {
      assert(0);
    }
  } else {
    value_type = type_empty;
  }
}
#endif

void ParameterValue::merge(const shared_ptr<ParameterValue> &parametervalue) {
  assert(parameter()->name() == parametervalue->parameter()->name());
  // Cannot insert "configurations" since configurations have not been merged
  // yet
  if (value_type == type_empty) {
    value_type = parametervalue->value_type;
    switch (value_type) {
    case type_empty:
      break;
    case type_int:
      value_int = parametervalue->value_int;
      break;
    case type_double:
      value_double = parametervalue->value_double;
      break;
    case type_string:
      value_string = parametervalue->value_string;
      break;
    default:
      assert(0);
    }
  }
  bool isequal = value_type == parametervalue->value_type;
  if (isequal) {
    switch (value_type) {
    case type_empty:
      break;
    case type_int:
      isequal &= value_int == parametervalue->value_int;
      break;
    case type_double:
      isequal &= value_double == parametervalue->value_double;
      break;
    case type_string:
      isequal &= value_string == parametervalue->value_string;
      break;
    default:
      assert(0);
    }
  }
  if (!isequal) {
    ostringstream buf;
    buf << "Cannot merge differing ParameterValues:\n"
        << indent(1) << "current:\n";
    this->output(buf, 2);
    buf << indent(1) << "new:\n";
    parametervalue->output(buf, 2);
    throw range_error(buf.str());
  }
}

void ParameterValue::setValue() { value_type = type_empty; }
void ParameterValue::setValue(long long i) {
  value_int = i;
  value_type = type_int;
}
void ParameterValue::setValue(double d) {
  value_double = d;
  value_type = type_double;
}
void ParameterValue::setValue(const string &s) {
  value_string = s;
  value_type = type_string;
}

ParameterValue::value_type_t ParameterValue::getValueType() const {
  return value_type;
}
long long ParameterValue::getValueInt() const {
  assert(value_type == type_int);
  return value_int;
}
double ParameterValue::getValueDouble() const {
  assert(value_type == type_double);
  return value_double;
}
string ParameterValue::getValueString() const {
  assert(value_type == type_string);
  return value_string;
}

ostream &ParameterValue::output(ostream &os, int level) const {
  os << indent(level) << "ParameterValue " << quote(name()) << ": Parameter "
     << quote(parameter()->name()) << "\n"
     << indent(level + 1) << "value: ";
  switch (value_type) {
  case type_empty:
    os << "empty";
    break;
  case type_int:
    os << "int(" << value_int << ")";
    break;
  case type_double:
    os << "double(" << value_double << ")";
    break;
  case type_string:
    os << "string(" << quote(value_string) << ")";
    break;
  default:
    assert(0);
  }
  os << "\n";
  return os;
}

#ifdef SIMULATIONIO_HAVE_HDF5
void ParameterValue::write(const H5::H5Location &loc,
                           const H5::H5Location &parent) const {
  assert(invariant());
  auto group = loc.createGroup(name());
  H5::createAttribute(group, "type", parameter()->project()->enumtype,
                      "ParameterValue");
  H5::createAttribute(group, "name", name());
  // H5::createHardLink(group, "parameter", parent,
  //                    "project/parameters/" +
  //                    parameter.lock()->name());
  // H5::createHardLink(group, "parameter", parent, ".");
  H5::createHardLink(group, "..", parent, ".");
  H5::createSoftLink(group, "parameter", "..");
  switch (value_type) {
  case type_empty:
    // do nothing
    break;
  case type_int:
    H5::createAttribute(group, "data", value_int);
    break;
  case type_double:
    H5::createAttribute(group, "data", value_double);
    break;
  case type_string:
    if (value_string.size() <= 16384)
      H5::createAttribute(group, "data", value_string);
    else
      // Store large attributes in a dataset
      H5::createDataSet(group, "data", value_string);
    break;
  default:
    assert(0);
  }
  group.createGroup("configurations");
}
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
vector<string> ParameterValue::yaml_path() const {
  return concat(parameter()->yaml_path(), {"parametervalues", name()});
}

ASDF::writer &ParameterValue::write(ASDF::writer &w) const {
  auto aw = asdf_writer(w);
  switch (value_type) {
  case type_empty:
    // do nothing
    break;
  case type_int:
    // TODO: use tags !<tag:yaml.org,2002:str> [float|int|str] instead
    aw.value("type", "int");
    aw.value("data", value_int);
    break;
  case type_double:
    aw.value("type", "float");
    aw.value("data", value_double);
    break;
  case type_string:
    aw.value("type", "string");
    aw.value("data", value_string);
    break;
  default:
    assert(0);
  }
  return w;
}
#endif

void ParameterValue::insert(const shared_ptr<Configuration> &configuration) {
  assert(parameter()->project().get() == configuration->project().get());
  checked_emplace(m_configurations, configuration->name(), configuration,
                  "ParameterValue", "configurations");
}

} // namespace SimulationIO
