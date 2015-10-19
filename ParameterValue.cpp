#include "ParameterValue.hpp"

#include "Parameter.hpp"
#include "Configuration.hpp"

#include "H5Helpers.hpp"

namespace SimulationIO {

ParameterValue::ParameterValue(const H5::CommonFG &loc, const string &entry,
                               Parameter *parameter)
    : parameter(parameter), value_type(type_empty) {
  auto group = loc.openGroup(entry);
  assert(H5::readAttribute<string>(
             group, "type", parameter->project->enumtype) == "ParameterValue");
  H5::readAttribute(group, "name", name);
  assert(H5::readGroupAttribute<string>(group, "parameter", "name") ==
         parameter->name);
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
  }
  // Cannot check "configurations" since configurations have not been read yet
  // assert(H5::checkGroupNames(group, "configurations", configurations));
}

void ParameterValue::setValue() { value_type = type_empty; }
void ParameterValue::setValue(int i) {
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

ostream &ParameterValue::output(ostream &os, int level) const {
  os << indent(level) << "ParameterValue \"" << name << "\": Parameter \""
     << parameter->name << "\"\n" << indent(level + 1) << "value: ";
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
    os << "string(\"" << value_string << "\")";
    break;
  default:
    assert(0);
  }
  os << "\n";
  return os;
}

void ParameterValue::write(const H5::CommonFG &loc,
                           const H5::H5Location &parent) const {
  auto group = loc.createGroup(name);
  H5::createAttribute(group, "type", parameter->project->enumtype,
                      "ParameterValue");
  H5::createAttribute(group, "name", name);
  H5::createHardLink(group, "parameter", parent,
                     string("project/parameters/") + parameter->name);
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
    H5::createAttribute(group, "data", value_string);
    break;
  default:
    assert(0);
  }
  group.createGroup("configurations");
}

void ParameterValue::insert(Configuration *configuration) {
  checked_emplace(configurations, configuration->name, configuration);
}
}
