#include "BasisVector.hpp"

#include "H5Helpers.hpp"

namespace SimulationIO {

void BasisVector::read(const H5::CommonFG &loc, const string &entry,
                       const shared_ptr<Basis> &basis) {
  this->basis = basis;
  auto group = loc.openGroup(entry);
  assert(H5::readAttribute<string>(
             group, "type",
             basis->tangentspace.lock()->project.lock()->enumtype) ==
         "BasisVector");
  H5::readAttribute(group, "name", name);
  assert(H5::readGroupAttribute<string>(group, "basis", "name") == basis->name);
  H5::readAttribute(group, "direction", direction);
}

ostream &BasisVector::output(ostream &os, int level) const {
  os << indent(level) << "BasisVector " << quote(name) << ": Basis "
     << quote(basis.lock()->name) << " direction=" << direction << "\n";
  return os;
}

void BasisVector::write(const H5::CommonFG &loc,
                        const H5::H5Location &parent) const {
  assert(invariant());
  auto group = loc.createGroup(name);
  H5::createAttribute(
      group, "type",
      basis.lock()->tangentspace.lock()->project.lock()->enumtype,
      "BasisVector");
  H5::createAttribute(group, "name", name);
  H5::createHardLink(group, "basis", parent, ".");
  H5::createAttribute(group, "direction", direction);
}
}
