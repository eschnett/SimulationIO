#include "Manifold.hpp"

#include "CoordinateSystem.hpp"
#include "Discretization.hpp"
#include "Field.hpp"
#include "SubDiscretization.hpp"

#ifdef SIMULATIONIO_HAVE_HDF5
#include "H5Helpers.hpp"
#endif

#include <algorithm>
#include <set>

namespace SimulationIO {

using std::equal;
using std::set;

bool Manifold::invariant() const {
  bool inv = Common::invariant() && bool(project()) &&
             project()->manifolds().count(name()) &&
             project()->manifolds().at(name()).get() == this &&
             bool(configuration()) &&
             configuration()->manifolds().count(name()) &&
             configuration()->manifolds().at(name()).lock().get() == this &&
             dimension() >= 0;
  for (const auto &d : discretizations())
    inv &= !d.first.empty() && bool(d.second);
  return inv;
}

#ifdef SIMULATIONIO_HAVE_HDF5
void Manifold::read(const H5::H5Location &loc, const string &entry,
                    const shared_ptr<Project> &project) {
  m_project = project;
  auto group = loc.openGroup(entry);
  assert(H5::readAttribute<string>(group, "type", project->enumtype) ==
         "Manifold");
  H5::readAttribute(group, "name", m_name);
  assert(H5::readGroupAttribute<string>(group, "project", "name") ==
         project->name());
  m_configuration = project->configurations().at(
      H5::readGroupAttribute<string>(group, "configuration", "name"));
  assert(H5::readGroupAttribute<string>(
             group, "configuration/manifolds/" + name(), "name") == name());
  H5::readAttribute(group, "dimension", m_dimension);
  H5::readGroup(group, "discretizations",
                [&](const H5::Group &group, const string &name) {
                  readDiscretization(group, name);
                });
  H5::readGroup(group, "subdiscretizations",
                [&](const H5::Group &group, const string &name) {
                  readSubDiscretization(group, name);
                });
  // Cannot check "fields", "coordinatesystems" since they have not been read
  // yet
  // assert(H5::checkGroupNames(group, "fields", fields));
  // assert(H5::checkGroupNames(group, "coordinatesystems", fields));
  m_configuration->insert(name(), shared_from_this());
}
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
void Manifold::read(const shared_ptr<ASDF::reader_state> &rs,
                    const YAML::Node &node,
                    const shared_ptr<Project> &project) {
  assert(node.Tag() ==
         "tag:github.com/eschnett/SimulationIO/asdf-cxx/Manifold-1.0.0");
  m_name = node["name"].Scalar();
  m_project = project;
  m_configuration = project->getConfiguration(rs, node["configuration"]);
  m_dimension = node["dimension"].as<int>();
  for (const auto &kv : node["discretizations"])
    readDiscretization(rs, kv.second);
  for (const auto &kv : node["subdiscretizations"])
    readSubDiscretization(rs, kv.second);
  m_configuration->insert(name(), shared_from_this());
}
#endif

#ifdef SIMULATIONIO_HAVE_SILO
void Manifold::read(const Silo<DBfile> &file, const string &loc,
                    const shared_ptr<Project> &project) {
  read_attribute(m_name, file, loc, "name");
  m_project = project;
  const auto &configuration_name =
      read_symlinked_name(file, loc, "configuration");
  m_configuration = project->configurations().at(configuration_name);
  read_attribute(m_dimension, file, loc, "dimension");
  read_group(file, loc, "discretizations",
             [&](const string &loc) { readDiscretization(file, loc); });
  read_group(file, loc, "subdiscretizations",
             [&](const string &loc) { readSubDiscretization(file, loc); });
  m_configuration->insert(name(), shared_from_this());
}
#endif

void Manifold::merge(const shared_ptr<Manifold> &manifold) {
  assert(project()->name() == manifold->project()->name());
  assert(m_configuration->name() == manifold->configuration()->name());
  assert(m_dimension == manifold->dimension());
  for (const auto &iter : manifold->discretizations()) {
    const auto &discretization = iter.second;
    if (!m_discretizations.count(discretization->name()))
      createDiscretization(discretization->name(),
                           project()->configurations().at(
                               discretization->configuration()->name()));
    m_discretizations.at(discretization->name())->merge(discretization);
  }
  for (const auto &iter : manifold->subdiscretizations()) {
    const auto &subdiscretization = iter.second;
    if (!m_subdiscretizations.count(subdiscretization->name()))
      createSubDiscretization(
          subdiscretization->name(),
          discretizations().at(
              subdiscretization->parent_discretization()->name()),
          discretizations().at(
              subdiscretization->child_discretization()->name()),
          subdiscretization->factor(), subdiscretization->offset());
    m_subdiscretizations.at(subdiscretization->name())
        ->merge(subdiscretization);
  }
}

ostream &Manifold::output(ostream &os, int level) const {
  os << indent(level) << "Manifold " << quote(name()) << ": Configuration "
     << quote(configuration()->name()) << " dim=" << dimension() << "\n";
  for (const auto &d : discretizations())
    d.second->output(os, level + 1);
  for (const auto &sd : subdiscretizations())
    sd.second->output(os, level + 1);
  for (const auto &f : fields())
    os << indent(level + 1) << "Field " << quote(f.second.lock()->name())
       << "\n";
  for (const auto &cs : coordinatesystems())
    os << indent(level + 1) << "CoordinateSystem "
       << quote(cs.second.lock()->name()) << "\n";
  return os;
}

#ifdef SIMULATIONIO_HAVE_HDF5
void Manifold::write(const H5::H5Location &loc,
                     const H5::H5Location &parent) const {
  assert(invariant());
  auto group = loc.createGroup(name());
  H5::createAttribute(group, "type", project()->enumtype, "Manifold");
  H5::createAttribute(group, "name", name());
  // H5::createHardLink(group, "project", parent, ".");
  H5::createHardLink(group, "..", parent, ".");
  H5::createSoftLink(group, "project", "..");
  // H5::createHardLink(group, "configuration", parent,
  //                    "configurations/" + configuration->name());
  H5::createSoftLink(group, "configuration",
                     "../configurations/" + configuration()->name());
  H5::createHardLink(
      group, "project/configurations/" + configuration()->name() + "/manifolds",
      name(), group, ".");
  H5::createAttribute(group, "dimension", dimension());
  H5::createGroup(group, "discretizations", discretizations());
  H5::createGroup(group, "subdiscretizations", subdiscretizations());
  group.createGroup("fields");
  group.createGroup("coordinatesystems");
}
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
vector<string> Manifold::yaml_path() const {
  return concat(project()->yaml_path(), {"manifolds", name()});
}

ASDF::writer &Manifold::write(ASDF::writer &w) const {
  auto aw = asdf_writer(w);
  aw.alias("configuration", *configuration());
  aw.value("dimension", dimension());
  aw.group("discretizations", discretizations());
  aw.group("subdiscretizations", subdiscretizations());
  return w;
}
#endif

#ifdef SIMULATIONIO_HAVE_SILO
string Manifold::silo_path() const {
  return project()->silo_path() + "manifolds/" + legalize_silo_name(name()) +
         "/";
}

void Manifold::write(const Silo<DBfile> &file, const string &loc) const {
  assert(invariant());
  write_attribute(file, loc, "name", name());
  write_symlink(file, loc, "configuration", configuration()->silo_path());
  write_attribute(file, loc, "dimension", dimension());
  write_group(file, loc, "discretizations", discretizations());
  write_group(file, loc, "subdiscretizations", subdiscretizations());
}
#endif

#ifdef SIMULATIONIO_HAVE_TILEDB
vector<string> Manifold ::tiledb_path() const {
  return concat(project()->tiledb_path(), {"manifolds", name()});
}

void Manifold::write(const tiledb::Context &ctx, const string &loc) const {
  assert(invariant());
  const tiledb_writer w(*this, ctx, loc);
  w.add_symlink(concat(tiledb_path(), {"configuration"}),
                configuration()->tiledb_path());
  w.add_symlink(concat(configuration()->tiledb_path(), {"manifolds", name()}),
                tiledb_path());
  w.add_attribute("dimension", dimension());
  w.add_group("discretizations", discretizations());
  w.add_group("subdiscretizations", subdiscretizations());
  w.create_group("fields");
  w.create_group("coordinatesystems");
}
#endif

shared_ptr<Discretization>
Manifold::createDiscretization(const string &name,
                               const shared_ptr<Configuration> &configuration) {
  assert(configuration->project().get() == project().get());
  auto discretization =
      Discretization::create(name, shared_from_this(), configuration);
  checked_emplace(m_discretizations, discretization->name(), discretization,
                  "Manifold", "discretizations");
  assert(discretization->invariant());
  return discretization;
}

shared_ptr<Discretization>
Manifold::getDiscretization(const string &name,
                            const shared_ptr<Configuration> &configuration) {
  auto loc = m_discretizations.find(name);
  if (loc != m_discretizations.end()) {
    const auto &discretization = loc->second;
    assert(discretization->configuration() == configuration);
    return discretization;
  }
  return createDiscretization(name, configuration);
}

shared_ptr<Discretization>
Manifold::copyDiscretization(const shared_ptr<Discretization> &discretization,
                             bool copy_children) {
  auto configuration2 =
      project()->copyConfiguration(discretization->configuration());
  auto discretization2 =
      getDiscretization(discretization->name(), configuration2);
  if (copy_children) {
    for (const auto &discretizationblock_kv :
         discretization->discretizationblocks()) {
      const auto &discretizationblock = discretizationblock_kv.second;
      auto discretizationblock2 = discretization2->copyDiscretizationBlock(
          discretizationblock, copy_children);
    }
  }
  return discretization2;
}

#ifdef SIMULATIONIO_HAVE_HDF5
shared_ptr<Discretization>
Manifold::readDiscretization(const H5::H5Location &loc, const string &entry) {
  auto discretization = Discretization::create(loc, entry, shared_from_this());
  checked_emplace(m_discretizations, discretization->name(), discretization,
                  "Manifold", "discretizations");
  assert(discretization->invariant());
  return discretization;
}
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
shared_ptr<Discretization>
Manifold::readDiscretization(const shared_ptr<ASDF::reader_state> &rs,
                             const YAML::Node &node) {
  auto discretization = Discretization::create(rs, node, shared_from_this());
  checked_emplace(m_discretizations, discretization->name(), discretization,
                  "Manifold", "discretizations");
  assert(discretization->invariant());
  return discretization;
}

shared_ptr<Discretization>
Manifold::getDiscretization(const shared_ptr<ASDF::reader_state> &rs,
                            const YAML::Node &node) {
  auto ref = ASDF::reference(rs, node);
  auto doc_path = ref.get_split_target();
  const auto &doc = doc_path.first;
  const auto &path = doc_path.second;
  assert(doc.empty());
  assert(path.at(0) == project()->name());
  assert(path.at(1) == "manifolds");
  assert(path.at(2) == name());
  assert(path.at(3) == "discretizations");
  const auto &discretization_name = path.at(4);
  return discretizations().at(discretization_name);
}
#endif

#ifdef SIMULATIONIO_HAVE_SILO
shared_ptr<Discretization>
Manifold::readDiscretization(const Silo<DBfile> &file, const string &loc) {
  auto discretization = Discretization::create(file, loc, shared_from_this());
  checked_emplace(m_discretizations, discretization->name(), discretization,
                  "Manifold", "discretizations");
  assert(discretization->invariant());
  return discretization;
}
#endif

shared_ptr<SubDiscretization> Manifold::createSubDiscretization(
    const string &name, const shared_ptr<Discretization> &parent_discretization,
    const shared_ptr<Discretization> &child_discretization,
    const vector<double> &factor, const vector<double> &offset) {
  assert(parent_discretization->manifold().get() == this);
  assert(child_discretization->manifold().get() == this);
  auto subdiscretization =
      SubDiscretization::create(name, shared_from_this(), parent_discretization,
                                child_discretization, factor, offset);
  checked_emplace(m_subdiscretizations, subdiscretization->name(),
                  subdiscretization, "Manifold", "subdiscretizations");
  assert(subdiscretization->invariant());
  return subdiscretization;
}

shared_ptr<SubDiscretization> Manifold::getSubDiscretization(
    const string &name, const shared_ptr<Discretization> &parent_discretization,
    const shared_ptr<Discretization> &child_discretization,
    const vector<double> &factor, const vector<double> &offset) {
  auto loc = m_subdiscretizations.find(name);
  if (loc != m_subdiscretizations.end()) {
    const auto &subdiscretization = loc->second;
    assert(subdiscretization->parent_discretization() == parent_discretization);
    assert(subdiscretization->child_discretization() == child_discretization);
    assert(subdiscretization->factor() == factor);
    assert(subdiscretization->offset() == offset);
    return subdiscretization;
  }
  return createSubDiscretization(name, parent_discretization,
                                 child_discretization, factor, offset);
}

shared_ptr<SubDiscretization> Manifold::copySubDiscretization(
    const shared_ptr<SubDiscretization> &subdiscretization,
    bool copy_children) {
  auto parent_discretization2 =
      copyDiscretization(subdiscretization->parent_discretization());
  auto child_discretization2 =
      copyDiscretization(subdiscretization->child_discretization());
  auto subdiscretization2 = getSubDiscretization(
      subdiscretization->name(), parent_discretization2, child_discretization2,
      subdiscretization->factor(), subdiscretization->offset());
  return subdiscretization2;
}

#ifdef SIMULATIONIO_HAVE_HDF5
shared_ptr<SubDiscretization>
Manifold::readSubDiscretization(const H5::H5Location &loc,
                                const string &entry) {
  auto subdiscretization =
      SubDiscretization::create(loc, entry, shared_from_this());
  checked_emplace(m_subdiscretizations, subdiscretization->name(),
                  subdiscretization, "Manifold", "subdiscretizations");
  assert(subdiscretization->invariant());
  return subdiscretization;
}
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
shared_ptr<SubDiscretization>
Manifold::readSubDiscretization(const shared_ptr<ASDF::reader_state> &rs,
                                const YAML::Node &node) {
  auto subdiscretization =
      SubDiscretization::create(rs, node, shared_from_this());
  checked_emplace(m_subdiscretizations, subdiscretization->name(),
                  subdiscretization, "Manifold", "subdiscretizations");
  assert(subdiscretization->invariant());
  return subdiscretization;
}
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
shared_ptr<SubDiscretization>
Manifold::readSubDiscretization(const Silo<DBfile> &file, const string &loc) {
  auto subdiscretization =
      SubDiscretization::create(file, loc, shared_from_this());
  checked_emplace(m_subdiscretizations, subdiscretization->name(),
                  subdiscretization, "Manifold", "subdiscretizations");
  assert(subdiscretization->invariant());
  return subdiscretization;
}
#endif

} // namespace SimulationIO
