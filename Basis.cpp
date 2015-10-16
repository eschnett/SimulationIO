#include "Basis.hpp"

#include "BasisVector.hpp"

#include "H5Helpers.hpp"

namespace SimulationIO {

Basis::Basis(const H5::CommonFG &loc, const string &entry,
             TangentSpace *tangentspace)
    : tangentspace(tangentspace) {
  auto group = loc.openGroup(entry);
  string type;
  H5::readAttribute(group, "type", tangentspace->project->enumtype, type);
  assert(type == "Basis");
  H5::readAttribute(group, "name", name);
  // TODO: check link "tangentspace"
  H5::readGroup(
      group, "basisvectors", [&](const string &name, const H5::Group &group) {
        createBasisVector(group, name);
      }, basisvectors);
  // TODO: check "directions"
}

ostream &Basis::output(ostream &os, int level) const {
  os << indent(level) << "Basis \"" << name << "\": tangentspace=\""
     << tangentspace->name << "\"\n";
  for (const auto &db : directions)
    db.second->output(os, level + 1);
  return os;
}

void Basis::write(const H5::CommonFG &loc, const H5::H5Location &parent) const {
  auto group = loc.createGroup(name);
  H5::createAttribute(group, "type", tangentspace->project->enumtype, "Basis");
  H5::createAttribute(group, "name", name);
  H5::createHardLink(group, "tangentspace", parent, ".");
  // H5::createAttribute(group, "tangentspace", parent, ".");
  H5::createGroup(group, "basisvectors", basisvectors);
#warning "TODO: output directions"
}

BasisVector *Basis::createBasisVector(const string &name, int direction) {
  auto basisvector = new BasisVector(name, this, direction);
  checked_emplace(basisvectors, basisvector->name, basisvector);
  checked_emplace(directions, basisvector->direction, basisvector);
  assert(basisvector->invariant());
  return basisvector;
}

BasisVector *Basis::createBasisVector(const H5::CommonFG &loc,
                                      const string &entry) {
  auto basisvector = new BasisVector(loc, entry, this);
  checked_emplace(basisvectors, basisvector->name, basisvector);
  checked_emplace(directions, basisvector->direction, basisvector);
  assert(basisvector->invariant());
  return basisvector;
}
}
