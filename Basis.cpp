#include "Basis.hpp"

#include "BasisVector.hpp"

#include "H5Helpers.hpp"

namespace SimulationIO {

void Basis::read(const H5::CommonFG &loc, const string &entry,
                 const shared_ptr<TangentSpace> &tangentspace) {
  this->tangentspace = tangentspace;
  auto group = loc.openGroup(entry);
  assert(H5::readAttribute<string>(
             group, "type", tangentspace->project.lock()->enumtype) == "Basis");
  H5::readAttribute(group, "name", name);
  assert(H5::readGroupAttribute<string>(group, "tangentspace", "name") ==
         tangentspace->name);
  H5::readGroup(group, "basisvectors",
                [&](const H5::Group &group, const string &name) {
                  createBasisVector(group, name);
                });
#warning "TODO: check group directions"
}

ostream &Basis::output(ostream &os, int level) const {
  os << indent(level) << "Basis " << quote(name) << ": TangentSpace "
     << quote(tangentspace.lock()->name) << "\n";
  for (const auto &db : directions)
    db.second->output(os, level + 1);
  return os;
}

void Basis::write(const H5::CommonFG &loc, const H5::H5Location &parent) const {
  assert(invariant());
  auto group = loc.createGroup(name);
  H5::createAttribute(group, "type",
                      tangentspace.lock()->project.lock()->enumtype, "Basis");
  H5::createAttribute(group, "name", name);
  H5::createHardLink(group, "tangentspace", parent, ".");
  H5::createGroup(group, "basisvectors", basisvectors);
#warning "TODO: output directions"
}

shared_ptr<BasisVector> Basis::createBasisVector(const string &name,
                                                 int direction) {
  auto basisvector = BasisVector::create(name, shared_from_this(), direction);
  checked_emplace(basisvectors, basisvector->name, basisvector);
  checked_emplace(directions, basisvector->direction, basisvector);
  assert(basisvector->invariant());
  return basisvector;
}

shared_ptr<BasisVector> Basis::createBasisVector(const H5::CommonFG &loc,
                                                 const string &entry) {
  auto basisvector = BasisVector::create(loc, entry, shared_from_this());
  checked_emplace(basisvectors, basisvector->name, basisvector);
  checked_emplace(directions, basisvector->direction, basisvector);
  assert(basisvector->invariant());
  return basisvector;
}
}
