#include "TensorComponent.hpp"

#include "H5Helpers.hpp"

namespace SimulationIO {

TensorComponent::TensorComponent(const H5::CommonFG &loc, const string &entry,
                                 TensorType *tensortype)
    : tensortype(tensortype) {
  auto group = loc.openGroup(entry);
  string type;
  H5::readAttribute(group, "type", type);
  assert(type == "TensorComponent");
  H5::readAttribute(group, "name", name);
  // TODO: check link "tensortype"
  H5::readAttribute(group, "storage_index", storage_index);
  H5::readAttribute(group, "indexvalues", indexvalues);
}

ostream &TensorComponent::output(ostream &os, int level) const {
  os << indent(level) << "TensorComponent \"" << name << "\": tensortype=\""
     << tensortype->name << "\" storage_index=" << storage_index
     << " indexvalues=[";
  for (int i = 0; i < int(indexvalues.size()); ++i) {
    if (i > 0)
      os << ",";
    os << indexvalues[i];
  }
  os << "]\n";
  return os;
}

void TensorComponent::write(const H5::CommonFG &loc,
                            const H5::H5Location &parent) const {
  auto group = loc.createGroup(name);
  H5::createAttribute(group, "type", "TensorComponent");
  H5::createAttribute(group, "name", name);
  H5::createAttribute(group, "tensortype", parent, ".");
  H5::createAttribute(group, "storage_index", storage_index);
  H5::createAttribute(group, "indexvalues", indexvalues);
}
}
