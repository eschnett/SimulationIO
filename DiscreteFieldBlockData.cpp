#include "DiscreteFieldBlockData.hpp"

#include "H5Helpers.hpp"

namespace SimulationIO {

DiscreteFieldBlockData::DiscreteFieldBlockData(
    const H5::CommonFG &loc, const string &entry,
    DiscreteFieldBlock *discretefieldblock)
    : discretefieldblock(discretefieldblock) {
  auto group = loc.openGroup(entry);
  string type;
  H5::readAttribute(group, "type", type);
  assert(type == "DiscreteFieldBlockData");
  H5::readAttribute(group, "name", name);
  // TODO: check link "discretefieldblock"
  // TODO: Read and interpret objects (shallowly) instead of naively only
  // looking at their names
  {
    auto obj = group.openGroup("tensorcomponent");
    // H5::Group obj;
    // H5::readAttribute(group, "tensorcomponent", obj);
    string name;
    auto attr = H5::readAttribute(obj, "name", name);
    tensorcomponent = discretefieldblock->discretefield->field->tensortype
                          ->tensorcomponents.at(name);
  }
  // tensorcomponent->insert(this);
}

ostream &DiscreteFieldBlockData::output(ostream &os, int level) const {
  os << indent(level) << "DiscreteFieldBlockData \"" << name
     << "\": discretefieldblock=\"" << discretefieldblock->name
     << "\" tensorcomponent=\"" << tensorcomponent->name << "\"\n";
  // TODO: output information about the group, e.g. type and shape
  return os;
}

void DiscreteFieldBlockData::write(const H5::CommonFG &loc,
                                   const H5::H5Location &parent) const {
// TODO: Choose type and shape
#warning "TODO: create DataSet instead"
  auto group = loc.createGroup(name);
  H5::createAttribute(group, "type", "DiscreteFieldBlockData");
  H5::createAttribute(group, "name", name);
  H5::createHardLink(group, "discretefieldblock", parent, ".");
  H5::createHardLink(
      group, "tensorcomponent", parent,
      string("discretefield/field/tensortype/tensorcomponents/") +
          tensorcomponent->name);
  // H5::createAttribute(group, "discretefieldblock", parent, ".");
  // H5::createAttribute(group, "tensorcomponent", parent,
  //                     string("discretefield/field/tensortype/") +
  //                         tensorcomponent->name);
}
}
