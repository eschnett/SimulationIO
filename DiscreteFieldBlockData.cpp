#include "DiscreteFieldBlockData.hpp"

#include "H5Helpers.hpp"

namespace SimulationIO {

DiscreteFieldBlockData::DiscreteFieldBlockData(
    const H5::CommonFG &loc, const string &entry,
    DiscreteFieldBlock *discretefieldblock)
    : discretefieldblock(discretefieldblock), have_extlink(false) {
  auto group = loc.openGroup(entry);
  string type;
  H5::readAttribute(group, "type",
                    discretefieldblock->discretefield->field->project->enumtype,
                    type);
  assert(type == "DiscreteFieldBlockData");
  H5::readAttribute(group, "name", name);
#warning "TODO: check link discretefieldblock"
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
  H5::readExternalLink(group, "data", have_extlink, extlink_file_name,
                       extlink_obj_name);
  tensorcomponent->noinsert(this);
}

void DiscreteFieldBlockData::setExternalLink(const string &file_name,
                                             const string &obj_name) {
  have_extlink = true;
  extlink_file_name = file_name;
  extlink_obj_name = obj_name;
}

ostream &DiscreteFieldBlockData::output(ostream &os, int level) const {
  os << indent(level) << "DiscreteFieldBlockData \"" << name
     << "\": discretefieldblock=\"" << discretefieldblock->name
     << "\" tensorcomponent=\"" << tensorcomponent->name << "\"\n";
  if (have_extlink)
    os << indent(level + 1) << "data: external link \"" << extlink_file_name
       << "\", \"" << extlink_file_name << "\"\n";
  // TODO: output information about the dataset, e.g. type and shape
  return os;
}

void DiscreteFieldBlockData::write(const H5::CommonFG &loc,
                                   const H5::H5Location &parent) const {
  auto group = loc.createGroup(name);
  H5::createAttribute(
      group, "type",
      discretefieldblock->discretefield->field->project->enumtype,
      "DiscreteFieldBlockData");
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
  if (have_extlink)
    H5::createExternalLink(group, "data", extlink_file_name, extlink_obj_name);
}
}
