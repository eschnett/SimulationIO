#include "Field.hpp"

#include "DiscreteField.hpp"

#include "H5Helpers.hpp"

namespace SimulationIO {

bool Field::invariant() const {
  bool inv = Common::invariant() && bool(project()) &&
             project()->fields().count(name()) &&
             project()->fields().at(name()).get() == this &&
             bool(configuration()) && configuration()->fields().count(name()) &&
             configuration()->fields().at(name()).lock().get() == this &&
             bool(manifold()) && manifold()->fields().count(name()) &&
             manifold()->fields().at(name()).lock().get() == this &&
             bool(tangentspace()) && tangentspace()->fields().count(name()) &&
             tangentspace()->fields().at(name()).lock().get() == this &&
             bool(tensortype()) &&
             tangentspace()->dimension() == tensortype()->dimension() &&
             tensortype()->fields().nobacklink();
  for (const auto &df : discretefields())
    inv &= !df.first.empty() && bool(df.second);
  return inv;
}

void Field::read(const H5::H5Location &loc, const string &entry,
                 const shared_ptr<Project> &project) {
  m_project = project;
  auto group = loc.openGroup(entry);
  assert(H5::readAttribute<string>(group, "type", project->enumtype) ==
         "Field");
  H5::readAttribute(group, "name", m_name);
  assert(H5::readGroupAttribute<string>(group, "project", "name") ==
         project->name());
  // TODO: Read and interpret objects (shallowly) instead of naively only
  // looking at their names
  m_manifold = project->manifolds().at(
      H5::readGroupAttribute<string>(group, "manifold", "name"));
  m_configuration = project->configurations().at(
      H5::readGroupAttribute<string>(group, "configuration", "name"));
  assert(H5::readGroupAttribute<string>(group, "configuration/fields/" + name(),
                                        "name") == name());
  assert(H5::readGroupAttribute<string>(group, "manifold/fields/" + name(),
                                        "name") == name());
  m_tangentspace = project->tangentspaces().at(
      H5::readGroupAttribute<string>(group, "tangentspace", "name"));
  assert(H5::readGroupAttribute<string>(group, "tangentspace/fields/" + name(),
                                        "name") == name());
  m_tensortype = project->tensortypes().at(
      H5::readGroupAttribute<string>(group, "tensortype", "name"));
  H5::readGroup(group, "discretefields",
                [&](const H5::Group &group, const string &name) {
                  readDiscreteField(group, name);
                });
  m_configuration->insert(name(), shared_from_this());
  m_manifold->insert(name(), shared_from_this());
  m_tangentspace->insert(name(), shared_from_this());
  m_tensortype->noinsert(shared_from_this());
}

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
void Field::read(const ASDF::reader_state &rs, const YAML::Node &node,
                 const shared_ptr<Project> &project) {
  assert(node.Tag() ==
         "tag:github.com/eschnett/SimulationIO/asdf-cxx/Field-1.0.0");
  m_name = node["name"].Scalar();
  m_project = project;
  m_configuration = project->getConfiguration(rs, node["configuration"]);
  m_manifold = project->getManifold(rs, node["manifold"]);
  m_tangentspace = project->getTangentSpace(rs, node["tangentspace"]);
  m_tensortype = project->getTensorType(rs, node["tensortype"]);
  for (const auto &kv : node["discretefields"])
    readDiscreteField(rs, kv.second);
  m_configuration->insert(name(), shared_from_this());
  m_manifold->insert(name(), shared_from_this());
  m_tangentspace->insert(name(), shared_from_this());
  m_tensortype->noinsert(shared_from_this());
}
#endif

void Field::merge(const shared_ptr<Field> &field) {
  assert(project()->name() == field->project()->name());
  assert(m_configuration->name() == field->configuration()->name());
  assert(m_manifold->name() == field->manifold()->name());
  assert(m_tangentspace->name() == field->tangentspace()->name());
  assert(m_tensortype->name() == field->tensortype()->name());
  for (const auto &iter : field->discretefields()) {
    const auto &discretefield = iter.second;
    if (!m_discretefields.count(discretefield->name()))
      createDiscreteField(
          discretefield->name(),
          project()->configurations().at(
              discretefield->configuration()->name()),
          manifold()->discretizations().at(
              discretefield->discretization()->name()),
          tangentspace()->bases().at(discretefield->basis()->name()));
    m_discretefields.at(discretefield->name())->merge(discretefield);
  }
}

ostream &Field::output(ostream &os, int level) const {
  os << indent(level) << "Field " << quote(name()) << ": Configuration "
     << quote(configuration()->name()) << " Manifold "
     << quote(manifold()->name()) << " TangentSpace "
     << quote(tangentspace()->name()) << " TensorType "
     << quote(tensortype()->name()) << "\n";
  for (const auto &df : discretefields())
    df.second->output(os, level + 1);
  return os;
}

void Field::write(const H5::H5Location &loc,
                  const H5::H5Location &parent) const {
  assert(invariant());
  auto group = loc.createGroup(name());
  H5::createAttribute(group, "type", project()->enumtype, "Field");
  H5::createAttribute(group, "name", name());
  // H5::createHardLink(group, "project", parent, ".");
  H5::createHardLink(group, "..", parent, ".");
  H5::createSoftLink(group, "project", "..");
  // H5::createHardLink(group, "configuration", parent,
  //                    "configurations/" + configuration->name());
  H5::createSoftLink(group, "configuration",
                     "../configurations/" + configuration()->name());
  H5::createHardLink(
      group, "project/configurations/" + configuration()->name() + "/fields",
      name(), group, ".");
  // H5::createHardLink(group, "manifold", parent,
  //                    "manifolds/" + manifold->name());
  H5::createSoftLink(group, "manifold", "../manifolds/" + manifold()->name());
  H5::createHardLink(group,
                     "project/manifolds/" + manifold()->name() + "/fields",
                     name(), group, ".");
  // H5::createHardLink(group, "tangentspace", parent,
  //                    "tangentspaces/" + tangentspace->name());
  H5::createSoftLink(group, "tangentspace",
                     "../tangentspaces/" + tangentspace()->name());
  H5::createHardLink(
      group, "project/tangentspaces/" + tangentspace()->name() + "/fields",
      name(), group, ".");
  // H5::createHardLink(group, "tensortype", parent,
  //                    "tensortypes/" + tensortype->name());
  H5::createSoftLink(group, "tensortype",
                     "../tensortypes/" + tensortype()->name());
  H5::createGroup(group, "discretefields", discretefields());
}

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
vector<string> Field::yaml_path() const {
  return concat(project()->yaml_path(), {"fields", name()});
}

ASDF::writer &Field::write(ASDF::writer &w) const {
  auto aw = asdf_writer(w);
  aw.alias("configuration", *configuration());
  aw.alias("manifold", *manifold());
  aw.alias("tangentspace", *tangentspace());
  aw.alias("tensortype", *tensortype());
  aw.group("discretefields", discretefields());
  return w;
}
#endif

shared_ptr<DiscreteField>
Field::createDiscreteField(const string &name,
                           const shared_ptr<Configuration> &configuration,
                           const shared_ptr<Discretization> &discretization,
                           const shared_ptr<Basis> &basis) {
  assert(configuration->project().get() == project().get());
  assert(discretization->manifold().get() == manifold().get());
  assert(basis->tangentspace().get() == tangentspace().get());
  auto discretefield = DiscreteField::create(
      name, shared_from_this(), configuration, discretization, basis);
  checked_emplace(m_discretefields, discretefield->name(), discretefield,
                  "Field", "discretefields");
  assert(discretefield->invariant());
  return discretefield;
}

shared_ptr<DiscreteField>
Field::getDiscreteField(const string &name,
                        const shared_ptr<Configuration> &configuration,
                        const shared_ptr<Discretization> &discretization,
                        const shared_ptr<Basis> &basis) {
  auto loc = m_discretefields.find(name);
  if (loc != m_discretefields.end()) {
    const auto &discretefield = loc->second;
    assert(discretefield->configuration() == configuration);
    assert(discretefield->discretization() == discretization);
    assert(discretefield->basis() == basis);
    return discretefield;
  }
  return createDiscreteField(name, configuration, discretization, basis);
}

shared_ptr<DiscreteField>
Field::copyDiscreteField(const shared_ptr<DiscreteField> &discretefield,
                         bool copy_children) {
  auto configuration2 =
      project()->copyConfiguration(discretefield->configuration());
  auto discretization2 =
      manifold()->copyDiscretization(discretefield->discretization());
  auto basis2 = tangentspace()->copyBasis(discretefield->basis());
  auto discretefield2 = getDiscreteField(discretefield->name(), configuration2,
                                         discretization2, basis2);
  if (copy_children) {
    for (const auto &discretefieldblock_kv :
         discretefield->discretefieldblocks()) {
      const auto &discretefieldblock = discretefieldblock_kv.second;
      auto discretefieldblock2 = discretefield2->copyDiscreteFieldBlock(
          discretefieldblock, copy_children);
    }
  }
  return discretefield2;
}

shared_ptr<DiscreteField> Field::readDiscreteField(const H5::H5Location &loc,
                                                   const string &entry) {
  auto discretefield = DiscreteField::create(loc, entry, shared_from_this());
  checked_emplace(m_discretefields, discretefield->name(), discretefield,
                  "Field", "discretefields");
  assert(discretefield->invariant());
  return discretefield;
}

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
shared_ptr<DiscreteField> Field::readDiscreteField(const ASDF::reader_state &rs,
                                                   const YAML::Node &node) {
  auto discretefield = DiscreteField::create(rs, node, shared_from_this());
  checked_emplace(m_discretefields, discretefield->name(), discretefield,
                  "Field", "discretefields");
  assert(discretefield->invariant());
  return discretefield;
}
#endif

} // namespace SimulationIO
