#include "ParameterValue.hpp"

#include "Configuration.hpp"
#include "Parameter.hpp"

#include "H5Helpers.hpp"

namespace SimulationIO {

ParameterValue::ParameterValue(const H5::CommonFG &loc, const string &entry,
                               Configuration *configuration)
    : configuration(configuration) {
  auto group = loc.openGroup(entry);
  string type;
  H5::readAttribute(group, "type", configuration->project->enumtype, type);
  assert(type == "ParameterValue");
  H5::readAttribute(group, "name", name);
  // TODO: check link "configuration"
}

ostream &ParameterValue::output(ostream &os, int level) const {
  os << indent(level) << "ParameterValue \"" << name << "\": configuration=\""
     << configuration->name << "\" parameter=\"" << parameter->name << "\"\n";
  return os;
}

void ParameterValue::write(const H5::CommonFG &loc,
                           const H5::H5Location &parent) const {
  auto group = loc.createGroup(name);
  H5::createAttribute(group, "type", configuration->project->enumtype,
                      "ParameterValue");
  H5::createAttribute(group, "name", name);
  H5::createHardLink(group, "configuration", parent, ".");
  // H5::createAttribute(group, "configuration", parent, ".");
  H5::createHardLink(group, "parameter", parent,
                     string("project/parameters/") + parameter->name);
}
}
