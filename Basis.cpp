#include "Basis.hpp"

#include "BasisVector.hpp"

#ifdef SIMULATIONIO_HAVE_HDF5
#include "H5Helpers.hpp"
#endif

namespace SimulationIO {

bool Basis::invariant() const {
  return Common::invariant() && bool(tangentspace()) &&
         tangentspace()->bases().count(name()) &&
         tangentspace()->bases().at(name()).get() == this &&
         bool(configuration()) && configuration()->bases().count(name()) &&
         configuration()->bases().at(name()).lock().get() == this;
  // int(basisvectors.size()) == tangentspace->dimension
  // int(directions.size()) == tangentspace->dimension
}

#ifdef SIMULATIONIO_HAVE_HDF5
void Basis::read(const H5::H5Location &loc, const string &entry,
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
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
void Basis::read(const shared_ptr<ASDF::reader_state> &rs,
                 const YAML::Node &node,
                 const shared_ptr<TangentSpace> &tangentspace) {
  assert(node.Tag() ==
         "tag:github.com/eschnett/SimulationIO/asdf-cxx/Basis-1.0.0");
  m_name = node["name"].Scalar();
  m_tangentspace = tangentspace;
  m_configuration =
      tangentspace->project()->getConfiguration(rs, node["configuration"]);
  for (const auto &kv : node["basisvectors"])
    readBasisVector(rs, kv.second);
  // TODO: check group directions
  m_configuration->insert(name(), shared_from_this());
}
#endif

#ifdef SIMULATIONIO_HAVE_SILO
void Basis::read(const Silo<DBfile> &file, const string &loc,
                 const shared_ptr<TangentSpace> &tangentspace) {
  read_attribute(m_name, file, loc, "name");
  m_tangentspace = tangentspace;
  const auto &configuration_name =
      read_symlinked_name(file, loc, "configuration");
  m_configuration =
      tangentspace->project()->configurations().at(configuration_name);
  read_group(file, loc, "basisvectors",
             [&](const string &loc) { readBasisVector(file, loc); });
  m_configuration->insert(name(), shared_from_this());
}
#endif

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

#ifdef SIMULATIONIO_HAVE_HDF5
void Basis::write(const H5::H5Location &loc,
                  const H5::H5Location &parent) const {
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
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
vector<string> Basis::yaml_path() const {
  return concat(tangentspace()->yaml_path(), {"bases", name()});
}

ASDF::writer &Basis::write(ASDF::writer &w) const {
  auto aw = asdf_writer(w);
  aw.alias("configuration", *configuration());
  aw.group("basisvectors", basisvectors());
  aw.alias_group("directions", directions());
  return w;
}
#endif

#ifdef SIMULATIONIO_HAVE_SILO
string Basis::silo_path() const {
  return tangentspace()->silo_path() + "bases/" + legalize_silo_name(name()) +
         "/";
}

void Basis::write(const Silo<DBfile> &file, const string &loc) const {
  assert(invariant());
  write_attribute(file, loc, "name", name());
  write_symlink(file, loc, "configuration", configuration()->silo_path());
  write_group(file, loc, "basisvectors", basisvectors());
}
#endif

#ifdef SIMULATIONIO_HAVE_TILEDB
vector<string> Basis::tiledb_path() const {
  return concat(tangentspace()->tiledb_path(), {"bases", name()});
}

void Basis::write(const tiledb::Context &ctx, const string &loc) const {
  assert(invariant());
  const tiledb_writer w(*this, ctx, loc);
  w.add_symlink(concat(tiledb_path(), {"configuration"}),
                configuration()->tiledb_path());
  w.add_symlink(concat(configuration()->tiledb_path(), {"bases", name()}),
                tiledb_path());
  w.add_group("basisvectors", basisvectors());
  w.create_group("directions");
  for (const auto &kv : directions()) {
    const auto &dir = kv.first;
    const auto &direction = kv.second;
    string dir_string = to_string(dir);
    w.add_symlink(concat(tiledb_path(), {"directions", dir_string}),
                  direction->tiledb_path());
  }
}
#endif

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

shared_ptr<BasisVector> Basis::getBasisVector(const string &name,
                                              int direction) {
  auto loc = m_basisvectors.find(name);
  if (loc != m_basisvectors.end()) {
    const auto &basisvector = loc->second;
    assert(basisvector->direction() == direction);
    return basisvector;
  }
  return createBasisVector(name, direction);
}

shared_ptr<BasisVector>
Basis::copyBasisVector(const shared_ptr<BasisVector> &basisvector,
                       bool copy_children) {
  auto basisvector2 =
      getBasisVector(basisvector->name(), basisvector->direction());
  return basisvector2;
}

#ifdef SIMULATIONIO_HAVE_HDF5
shared_ptr<BasisVector> Basis::readBasisVector(const H5::H5Location &loc,
                                               const string &entry) {
  auto basisvector = BasisVector::create(loc, entry, shared_from_this());
  checked_emplace(m_basisvectors, basisvector->name(), basisvector, "Basis",
                  "basisvectors");
  checked_emplace(m_directions, basisvector->direction(), basisvector, "Basis",
                  "directions");
  assert(basisvector->invariant());
  return basisvector;
}
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
shared_ptr<BasisVector>
Basis::readBasisVector(const shared_ptr<ASDF::reader_state> &rs,
                       const YAML::Node &node) {
  auto basisvector = BasisVector::create(rs, node, shared_from_this());
  checked_emplace(m_basisvectors, basisvector->name(), basisvector, "Basis",
                  "basisvectors");
  checked_emplace(m_directions, basisvector->direction(), basisvector, "Basis",
                  "directions");
  assert(basisvector->invariant());
  return basisvector;
}
#endif

#ifdef SIMULATIONIO_HAVE_SILO
shared_ptr<BasisVector> Basis::readBasisVector(const Silo<DBfile> &file,
                                               const string &loc) {
  auto basisvector = BasisVector::create(file, loc, shared_from_this());
  checked_emplace(m_basisvectors, basisvector->name(), basisvector, "Basis",
                  "basisvectors");
  checked_emplace(m_directions, basisvector->direction(), basisvector, "Basis",
                  "directions");
  assert(basisvector->invariant());
  return basisvector;
}
#endif

} // namespace SimulationIO
