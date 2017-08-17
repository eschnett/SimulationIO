#include "Basis.hpp"

#include "BasisVector.hpp"

#include "H5Helpers.hpp"

namespace SimulationIO {

void Basis::read(const H5::CommonFG &loc, const string &entry,
                 const shared_ptr<TangentSpace> &tangentspace) {
  m_tangentspace = tangentspace;
  auto group = loc.openGroup(entry);
  assert(H5::readAttribute<string>(
             group, "type", tangentspace->project()->enumtype) == "Basis");
  H5::readAttribute(group, "name", m_name);
  assert(H5::readGroupAttribute<string>(group, "tangentspace", "name") ==
         tangentspace->name());
  m_configuration = tangentspace->project()->configurations().at(
      H5::readGroupAttribute<string>(group, "configuration", "name"));
  assert(H5::readGroupAttribute<string>(group, "configuration/bases/" + name(),
                                        "name") == name());
  H5::readGroup(group, "basisvectors",
                [&](const H5::Group &group, const string &name) {
                  readBasisVector(group, name);
                });
  // TODO: check group directions
  m_configuration->insert(name(), shared_from_this());
}

void Basis::merge(const shared_ptr<Basis> &basis) {
  assert(tangentspace()->name() == basis->tangentspace()->name());
  assert(m_configuration->name() == basis->configuration()->name());
  for (const auto &iter : basis->basisvectors()) {
    const auto &basisvector = iter.second;
    if (!m_basisvectors.count(basisvector->name()))
      createBasisVector(basisvector->name(), basisvector->direction());
    m_basisvectors.at(basisvector->name())->merge(basisvector);
  }
}

ostream &Basis::output(ostream &os, int level) const {
  os << indent(level) << "Basis " << quote(name()) << ": Configuration "
     << quote(configuration()->name()) << " TangentSpace "
     << quote(tangentspace()->name()) << "\n";
  for (const auto &db : directions())
    db.second->output(os, level + 1);
  return os;
}

void Basis::write(const H5::CommonFG &loc, const H5::H5Location &parent) const {
  assert(invariant());
  auto group = loc.createGroup(name());
  H5::createAttribute(group, "type", tangentspace()->project()->enumtype,
                      "Basis");
  H5::createAttribute(group, "name", name());
  // H5::createHardLink(group, "tangentspace", parent, ".");
  H5::createHardLink(group, "..", parent, ".");
  H5::createSoftLink(group, "tangentspace", "..");
  // H5::createHardLink(group, "configuration", parent,
  //                    "project/configurations/" +
  //                    configuration->name);
  H5::createSoftLink(group, "configuration",
                     "../project/configurations/" + configuration()->name());
  H5::createHardLink(group,
                     "tangentspace/project/configurations/" +
                         configuration()->name() + "/bases",
                     name(), group, ".");
  H5::createGroup(group, "basisvectors", basisvectors());
  // TODO: output directions
}

shared_ptr<BasisVector> Basis::createBasisVector(const string &name,
                                                 int direction) {
  auto basisvector = BasisVector::create(name, shared_from_this(), direction);
  checked_emplace(m_basisvectors, basisvector->name(), basisvector, "Basis",
                  "basisvectors");
  checked_emplace(m_directions, basisvector->direction(), basisvector, "Basis",
                  "directions");
  assert(basisvector->invariant());
  return basisvector;
}

shared_ptr<BasisVector> Basis::readBasisVector(const H5::CommonFG &loc,
                                               const string &entry) {
  auto basisvector = BasisVector::create(loc, entry, shared_from_this());
  checked_emplace(m_basisvectors, basisvector->name(), basisvector, "Basis",
                  "basisvectors");
  checked_emplace(m_directions, basisvector->direction(), basisvector, "Basis",
                  "directions");
  assert(basisvector->invariant());
  return basisvector;
}
}
