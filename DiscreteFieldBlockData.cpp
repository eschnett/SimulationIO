#include "DiscreteFieldBlockData.hpp"

#include "H5Helpers.hpp"

namespace SimulationIO {

void DiscreteFieldBlockData::read(
    const H5::CommonFG &loc, const string &entry,
    const shared_ptr<DiscreteFieldBlock> &discretefieldblock) {
  this->discretefieldblock = discretefieldblock;
  have_extlink = false;
  auto group = loc.openGroup(entry);
  assert(H5::readAttribute<string>(group, "type",
                                   discretefieldblock->discretefield.lock()
                                       ->field.lock()
                                       ->project.lock()
                                       ->enumtype) == "DiscreteFieldBlockData");
  H5::readAttribute(group, "name", name);
  assert(H5::readGroupAttribute<string>(group, "discretefieldblock", "name") ==
         discretefieldblock->name);
  // TODO: Read and interpret objects (shallowly) instead of naively only
  // looking at their names
  tensorcomponent =
      discretefieldblock->discretefield.lock()
          ->field.lock()
          ->tensortype->tensorcomponents.at(
              H5::readGroupAttribute<string>(group, "tensorcomponent", "name"));
  H5::readExternalLink(group, "data", have_extlink, extlink_file_name,
                       extlink_obj_name);
  tensorcomponent->noinsert(shared_from_this());
}

void DiscreteFieldBlockData::setExternalLink(const string &file_name,
                                             const string &obj_name) {
  have_extlink = true;
  extlink_file_name = file_name;
  extlink_obj_name = obj_name;
}

ostream &DiscreteFieldBlockData::output(ostream &os, int level) const {
  os << indent(level) << "DiscreteFieldBlockData \"" << name
     << "\": DiscreteFieldBlock \"" << discretefieldblock.lock()->name
     << "\" TensorComponent \"" << tensorcomponent->name << "\"\n";
  if (have_extlink)
    os << indent(level + 1) << "data: external link \"" << extlink_file_name
       << "\", \"" << extlink_file_name << "\"\n";
  return os;
}

void DiscreteFieldBlockData::write(const H5::CommonFG &loc,
                                   const H5::H5Location &parent) const {
  assert(invariant());
  auto group = loc.createGroup(name);
  H5::createAttribute(group, "type", discretefieldblock.lock()
                                         ->discretefield.lock()
                                         ->field.lock()
                                         ->project.lock()
                                         ->enumtype,
                      "DiscreteFieldBlockData");
  H5::createAttribute(group, "name", name);
  H5::createHardLink(group, "discretefieldblock", parent, ".");
  H5::createHardLink(
      group, "tensorcomponent", parent,
      string("discretefield/field/tensortype/tensorcomponents/") +
          tensorcomponent->name);
  if (have_extlink)
    H5::createExternalLink(group, "data", extlink_file_name, extlink_obj_name);
}
}
