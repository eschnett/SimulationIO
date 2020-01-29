#include "TangentSpace.hpp"

#include "Basis.hpp"
#include "Field.hpp"

#ifdef SIMULATIONIO_HAVE_HDF5
#include "H5Helpers.hpp"
#endif

namespace SimulationIO {

bool TangentSpace::invariant() const {
  bool inv = Common::invariant() && bool(project()) &&
             project()->tangentspaces().count(name()) &&
             project()->tangentspaces().at(name()).get() == this &&
             bool(configuration()) &&
             configuration()->tangentspaces().count(name()) &&
             configuration()->tangentspaces().at(name()).lock().get() == this &&
             dimension() >= 0;
  for (const auto &b : bases())
    inv &= !b.first.empty() && bool(b.second);
  return inv;
}

#ifdef SIMULATIONIO_HAVE_HDF5
void TangentSpace::read(const H5::H5Location &loc, const string &entry,
                        const shared_ptr<Project> &project) {
  m_project = project;
  auto group = loc.openGroup(entry);
  assert(H5::readAttribute<string>(group, "type", project->enumtype) ==
         "TangentSpace");
  H5::readAttribute(group, "name", m_name);
  assert(H5::readGroupAttribute<string>(group, "project", "name") ==
         project->name());
  m_configuration = project->configurations().at(
      H5::readGroupAttribute<string>(group, "configuration", "name"));
  assert(H5::readGroupAttribute<string>(
             group, "configuration/tangentspaces/" + name(), "name") == name());
  H5::readAttribute(group, "dimension", m_dimension);
  H5::readGroup(group, "bases",
                [&](const H5::Group &group, const string &name) {
                  readBasis(group, name);
                });
  // Cannot check "fields" since fields have not been read yet
  // assert(H5::checkGroupNames(group, "fields", fields));
  m_configuration->insert(name(), shared_from_this());
}
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
void TangentSpace::read(const shared_ptr<ASDF::reader_state> &rs,
                        const YAML::Node &node,
                        const shared_ptr<Project> &project) {
  assert(node.Tag() ==
         "tag:github.com/eschnett/SimulationIO/asdf-cxx/TangentSpace-1.0.0");
  m_name = node["name"].Scalar();
  m_project = project;
  m_configuration = project->getConfiguration(rs, node["configuration"]);
  m_dimension = node["dimension"].as<int>();
  for (const auto &kv : node["bases"])
    readBasis(rs, kv.second);
  m_configuration->insert(name(), shared_from_this());
}
#endif

void TangentSpace::merge(const shared_ptr<TangentSpace> &tangentspace) {
  assert(project()->name() == tangentspace->project()->name());
  assert(m_configuration->name() == tangentspace->configuration()->name());
  assert(m_dimension == tangentspace->dimension());
  for (const auto &iter : tangentspace->bases()) {
    const auto &basis = iter.second;
    if (!m_bases.count(basis->name()))
      createBasis(basis->name(), project()->configurations().at(
                                     basis->configuration()->name()));
    m_bases.at(basis->name())->merge(basis);
  }
}

ostream &TangentSpace::output(ostream &os, int level) const {
  os << indent(level) << "TangentSpace " << quote(name()) << ": Configuration "
     << quote(configuration()->name()) << " dim=" << dimension() << "\n";
  for (const auto &b : bases())
    b.second->output(os, level + 1);
  for (const auto &f : fields())
    os << indent(level + 1) << "Field " << quote(f.second.lock()->name())
       << "\n";
  return os;
}

#ifdef SIMULATIONIO_HAVE_HDF5
void TangentSpace::write(const H5::H5Location &loc,
                         const H5::H5Location &parent) const {
  assert(invariant());
  auto group = loc.createGroup(name());
  H5::createAttribute(group, "type", project()->enumtype, "TangentSpace");
  H5::createAttribute(group, "name", name());
  // H5::createHardLink(group, "project", parent, ".");
  H5::createHardLink(group, "..", parent, ".");
  H5::createSoftLink(group, "project", "..");
  // H5::createHardLink(group, "configuration", parent,
  //                    "configurations/" + configuration->name());
  H5::createSoftLink(group, "configuration",
                     "../configurations/" + configuration()->name());
  H5::createHardLink(group,
                     "project/configurations/" + configuration()->name() +
                         "/tangentspaces",
                     name(), group, ".");
  H5::createAttribute(group, "dimension", dimension());
  H5::createGroup(group, "bases", bases());
  group.createGroup("fields");
}
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
vector<string> TangentSpace::yaml_path() const {
  return concat(project()->yaml_path(), {"tangentspaces", name()});
}

ASDF::writer &TangentSpace::write(ASDF::writer &w) const {
  auto aw = asdf_writer(w);
  aw.alias("configuration", *configuration());
  aw.value("dimension", dimension());
  aw.group("bases", bases());
  return w;
}
#endif

#ifdef SIMULATIONIO_HAVE_SILO
string TangentSpace::silo_path() const {
  return project()->silo_path() + "tangentspaces/" +
         legalize_silo_name(name()) + "/";
}

void TangentSpace::write(DBfile *const file, const string &loc) const {
  assert(invariant());
  write_attribute(file, loc, "name", name());
  write_symlink(file, loc, "configuration", configuration()->silo_path());
  write_attribute(file, loc, "dimension", dimension());
  write_group(file, loc, "bases", bases());
}
#endif

#ifdef SIMULATIONIO_HAVE_TILEDB
vector<string> TangentSpace::tiledb_path() const {
  return concat(project()->tiledb_path(), {"tangentspaces", name()});
}

void TangentSpace::write(const tiledb::Context &ctx, const string &loc) const {
  assert(invariant());
  const tiledb_writer w(*this, ctx, loc);
  w.add_symlink(concat(tiledb_path(), {"configuration"}),
                configuration()->tiledb_path());
  w.add_symlink(
      concat(configuration()->tiledb_path(), {"tangentspaces", name()}),
      tiledb_path());
  w.add_attribute("dimension", dimension());
  w.add_group("bases", bases());
  w.create_group("fields");
}
#endif

shared_ptr<Basis>
TangentSpace::createBasis(const string &name,
                          const shared_ptr<Configuration> &configuration) {
  assert(configuration->project().get() == project().get());
  auto basis = Basis::create(name, shared_from_this(), configuration);
  checked_emplace(m_bases, basis->name(), basis, "TangentSpace", "bases");
  assert(basis->invariant());
  return basis;
}

shared_ptr<Basis>
TangentSpace::getBasis(const string &name,
                       const shared_ptr<Configuration> &configuration) {
  auto loc = m_bases.find(name);
  if (loc != m_bases.end()) {
    const auto &basis = loc->second;
    assert(basis->configuration() == configuration);
    return basis;
  }
  return createBasis(name, configuration);
}

shared_ptr<Basis> TangentSpace::copyBasis(const shared_ptr<Basis> &basis,
                                          bool copy_children) {
  auto configuration2 = project()->copyConfiguration(basis->configuration());
  auto basis2 = getBasis(basis->name(), configuration2);
  if (copy_children) {
    for (const auto &basisvector_kv : basis->basisvectors()) {
      const auto &basisvector = basisvector_kv.second;
      auto basisvector2 = basis2->copyBasisVector(basisvector, copy_children);
    }
  }
  return basis2;
}

#ifdef SIMULATIONIO_HAVE_HDF5
shared_ptr<Basis> TangentSpace::readBasis(const H5::H5Location &loc,
                                          const string &entry) {
  auto basis = Basis::create(loc, entry, shared_from_this());
  checked_emplace(m_bases, basis->name(), basis, "TangentSpace", "bases");
  assert(basis->invariant());
  return basis;
}
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
shared_ptr<Basis>
TangentSpace::readBasis(const shared_ptr<ASDF::reader_state> &rs,
                        const YAML::Node &node) {
  auto basis = Basis::create(rs, node, shared_from_this());
  checked_emplace(m_bases, basis->name(), basis, "TangentSpace", "bases");
  assert(basis->invariant());
  return basis;
}

shared_ptr<Basis>
TangentSpace::getBasis(const shared_ptr<ASDF::reader_state> &rs,
                       const YAML::Node &node) {
  auto ref = ASDF::reference(rs, node);
  auto doc_path = ref.get_split_target();
  const auto &doc = doc_path.first;
  const auto &path = doc_path.second;
  assert(doc.empty());
  assert(path.at(0) == project()->name());
  assert(path.at(1) == "tangentspaces");
  assert(path.at(2) == name());
  assert(path.at(3) == "bases");
  const auto &basis_name = path.at(4);
  return bases().at(basis_name);
}
#endif

} // namespace SimulationIO
