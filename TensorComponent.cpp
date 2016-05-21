#include "TensorComponent.hpp"

#include "H5Helpers.hpp"

namespace SimulationIO {

void TensorComponent::read(const H5::CommonFG &loc, const string &entry,
                           const shared_ptr<TensorType> &tensortype) {
  this->tensortype = tensortype;
  auto group = loc.openGroup(entry);
  assert(H5::readAttribute<string>(group, "type",
                                   tensortype->project.lock()->enumtype) ==
         "TensorComponent");
  H5::readAttribute(group, "name", m_name);
  assert(H5::readGroupAttribute<string>(group, "tensortype", "name") ==
         tensortype->name());
  H5::readAttribute(group, "storage_index", storage_index);
  H5::readAttribute(group, "indexvalues", indexvalues);
}

ostream &TensorComponent::output(ostream &os, int level) const {
  os << indent(level) << "TensorComponent " << quote(name()) << ": TensorType "
     << quote(tensortype.lock()->name()) << " storage_index=" << storage_index
     << " indexvalues=" << indexvalues << "\n";
  return os;
}

void TensorComponent::write(const H5::CommonFG &loc,
                            const H5::H5Location &parent) const {
  assert(invariant());
  auto group = loc.createGroup(name());
  H5::createAttribute(group, "type",
                      tensortype.lock()->project.lock()->enumtype,
                      "TensorComponent");
  H5::createAttribute(group, "name", name());
  // H5::createHardLink(group, "tensortype", parent, ".");
  H5::createHardLink(group, "..", parent, ".");
  H5::createSoftLink(group, "tensortype", "..");
  H5::createAttribute(group, "storage_index", storage_index);
  H5::createAttribute(group, "indexvalues", indexvalues);
}
}
