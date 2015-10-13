#include "BasisVector.hpp"

#include "H5Helpers.hpp"

namespace SimulationIO {

BasisVector::BasisVector(const H5::CommonFG &loc, const string &entry,
                         Basis *basis)
    : basis(basis) {
  auto group = loc.openGroup(entry);
  string type;
  H5::readAttribute(group, "type", type);
  assert(type == "BasisVector");
  H5::readAttribute(group, "name", name);
  // TODO: check link "basis"
  H5::readAttribute(group, "direction", direction);
}

ostream &BasisVector::output(ostream &os, int level) const {
  os << indent(level) << "BasisVector \"" << name << "\": basis=\""
     << basis->name << "\" direction=" << direction << "\n";
  return os;
}

void BasisVector::write(const H5::CommonFG &loc,
                        const H5::H5Location &parent) const {
  auto group = loc.createGroup(name);
  H5::createAttribute(group, "type", "BasisVector");
  H5::createAttribute(group, "name", name);
  H5::createHardLink(group, "basis", parent, ".");
  // H5::createAttribute(group, "basis", parent, ".");
  H5::createAttribute(group, "direction", direction);
}
}
