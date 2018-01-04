#include "BasisVector.hpp"

#include "H5Helpers.hpp"

namespace SimulationIO {

void BasisVector::read(const H5::H5Location &loc, const string &entry,
                       const shared_ptr<Basis> &basis) {
  m_basis = basis;
  auto group = loc.openGroup(entry);
  assert(H5::readAttribute<string>(
             group, "type", basis->tangentspace()->project()->enumtype) ==
         "BasisVector");
  H5::readAttribute(group, "name", m_name);
  assert(H5::readGroupAttribute<string>(group, "basis", "name") ==
         basis->name());
  H5::readAttribute(group, "direction", m_direction);
}

void BasisVector::merge(const shared_ptr<BasisVector> &basisvector) {
  assert(basis()->name() == basisvector->basis()->name());
  assert(m_direction == basisvector->direction());
}

ostream &BasisVector::output(ostream &os, int level) const {
  os << indent(level) << "BasisVector " << quote(name()) << ": Basis "
     << quote(basis()->name()) << " direction=" << direction() << "\n";
  return os;
}

void BasisVector::write(const H5::H5Location &loc,
                        const H5::H5Location &parent) const {
  assert(invariant());
  auto group = loc.createGroup(name());
  H5::createAttribute(group, "type",
                      basis()->tangentspace()->project()->enumtype,
                      "BasisVector");
  H5::createAttribute(group, "name", name());
  // H5::createHardLink(group, "basis", parent, ".");
  H5::createHardLink(group, "..", parent, ".");
  H5::createSoftLink(group, "basis", "..");
  H5::createAttribute(group, "direction", direction());
}
} // namespace SimulationIO
