#include "DiscreteField.hpp"

#include "DiscreteFieldBlock.hpp"

#ifdef SIMULATIONIO_HAVE_HDF5
#include "H5Helpers.hpp"
#endif

namespace SimulationIO {

bool DiscreteField::invariant() const {
  return Common::invariant() && bool(field()) &&
         field()->discretefields().count(name()) &&
         field()->discretefields().at(name()).get() == this &&
         bool(configuration()) &&
         configuration()->discretefields().count(name()) &&
         configuration()->discretefields().at(name()).lock().get() == this &&
         bool(discretization()) &&
         discretization()->discretefields().nobacklink() &&
         field()->manifold().get() == discretization()->manifold().get() &&
         bool(basis()) && basis()->discretefield().nobacklink() &&
         field()->tangentspace().get() == basis()->tangentspace().get();
}

#ifdef SIMULATIONIO_HAVE_HDF5
void DiscreteField::read(const H5::H5Location &loc, const string &entry,
                         const shared_ptr<Field> &field) {
  m_field = field;
  auto group = loc.openGroup(entry);
  assert(H5::readAttribute<string>(group, "type", field->project()->enumtype) ==
         "DiscreteField");
  H5::readAttribute(group, "name", m_name);
  assert(H5::readGroupAttribute<string>(group, "field", "name") ==
         field->name());
  // TODO: Read and interpret objects (shallowly) instead of naively only
  // looking at their names
  m_configuration = field->project()->configurations().at(
      H5::readGroupAttribute<string>(group, "configuration", "name"));
  assert(H5::readGroupAttribute<string>(
             group, "configuration/discretefields/" + name(), "name") ==
         name());
  m_discretization = field->manifold()->discretizations().at(
      H5::readGroupAttribute<string>(group, "discretization", "name"));
  m_basis = field->tangentspace()->bases().at(
      H5::readGroupAttribute<string>(group, "basis", "name"));
  H5::readGroup(group, "discretefieldblocks",
                [&](const H5::Group &group, const string &name) {
                  readDiscreteFieldBlock(group, name);
                });
  m_configuration->insert(name(), shared_from_this());
  m_discretization->noinsert(shared_from_this());
  m_basis->noinsert(shared_from_this());
}
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
void DiscreteField::read(const shared_ptr<ASDF::reader_state> &rs,
                         const YAML::Node &node,
                         const shared_ptr<Field> &field) {
  assert(node.Tag() ==
         "tag:github.com/eschnett/SimulationIO/asdf-cxx/DiscreteField-1.0.0");
  m_name = node["name"].Scalar();
  m_field = field;
  m_configuration =
      field->project()->getConfiguration(rs, node["configuration"]);
  m_discretization =
      field->manifold()->getDiscretization(rs, node["discretization"]);
  m_basis = field->tangentspace()->getBasis(rs, node["basis"]);
  for (const auto &kv : node["discretefieldblocks"])
    readDiscreteFieldBlock(rs, kv.second);
  m_configuration->insert(name(), shared_from_this());
  m_discretization->noinsert(shared_from_this());
  m_basis->noinsert(shared_from_this());
}
#endif

void DiscreteField::merge(const shared_ptr<DiscreteField> &discretefield) {
  assert(field()->name() == discretefield->field()->name());
  assert(m_configuration->name() == discretefield->configuration()->name());
  assert(m_discretization->name() == discretefield->discretization()->name());
  assert(m_basis->name() == discretefield->basis()->name());
  for (const auto &iter : discretefield->discretefieldblocks()) {
    const auto &discretefieldblock = iter.second;
    if (!m_discretefieldblocks.count(discretefieldblock->name()))
      createDiscreteFieldBlock(
          discretefieldblock->name(),
          m_discretization->discretizationblocks().at(
              discretefieldblock->discretizationblock()->name()));
    m_discretefieldblocks.at(discretefieldblock->name())
        ->merge(discretefieldblock);
  }
}

ostream &DiscreteField::output(ostream &os, int level) const {
  os << indent(level) << "DiscreteField " << quote(name()) << ": Configuration "
     << quote(configuration()->name()) << " Field " << quote(field()->name())
     << " Discretization " << quote(discretization()->name()) << " Basis "
     << quote(basis()->name()) << "\n";
  for (const auto &db : discretefieldblocks())
    db.second->output(os, level + 1);
  return os;
}

#ifdef SIMULATIONIO_HAVE_HDF5
void DiscreteField::write(const H5::H5Location &loc,
                          const H5::H5Location &parent) const {
  assert(invariant());
  auto group = loc.createGroup(name());
  H5::createAttribute(group, "type", field()->project()->enumtype,
                      "DiscreteField");
  H5::createAttribute(group, "name", name());
  // H5::createHardLink(group, "field", parent, ".");
  H5::createHardLink(group, "..", parent, ".");
  H5::createSoftLink(group, "field", "..");
  // H5::createHardLink(group, "configuration", parent,
  //                    "project/configurations/" +
  //                    configuration->name());
  H5::createSoftLink(group, "configuration",
                     "../project/configurations/" + configuration()->name());
  H5::createHardLink(group,
                     "field/project/configurations/" + configuration()->name() +
                         "/discretefields",
                     name(), group, ".");
  // H5::createHardLink(group, "discretization", parent,
  //                    "manifold/discretizations/" +
  //                        discretization->name());
  H5::createSoftLink(group, "discretization",
                     "../manifold/discretizations/" + discretization()->name());
  // H5::createHardLink(group, "basis", parent,
  //                    "tangentspace/bases/" + basis->name());
  H5::createSoftLink(group, "basis",
                     "../tangentspace/bases/" + basis()->name());
  createGroup(group, "discretefieldblocks", discretefieldblocks());
}
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
vector<string> DiscreteField::yaml_path() const {
  return concat(field()->yaml_path(), {"discretefields", name()});
}

ASDF::writer &DiscreteField::write(ASDF::writer &w) const {
  auto aw = asdf_writer(w);
  aw.alias("configuration", *configuration());
  aw.alias("discretization", *discretization());
  aw.alias("basis", *basis());
  aw.group("discretefieldblocks", discretefieldblocks());
  return w;
}
#endif

#ifdef SIMULATIONIO_HAVE_TILEDB
vector<string> DiscreteField::tiledb_path() const {
  return concat(field()->tiledb_path(), {"discretefields", name()});
}

void DiscreteField::write(const tiledb::Context &ctx, const string &loc) const {
  assert(invariant());
  const tiledb_writer w(*this, ctx, loc);
  w.add_symlink(concat(tiledb_path(), {"configuration"}),
                configuration()->tiledb_path());
  w.add_symlink(
      concat(configuration()->tiledb_path(), {"discretefields", name()}),
      tiledb_path());
  w.add_symlink(concat(tiledb_path(), {"discretization"}),
                discretization()->tiledb_path());
  w.add_symlink(concat(tiledb_path(), {"basis"}), basis()->tiledb_path());
  w.add_group("discretefieldblocks", discretefieldblocks());
}
#endif

shared_ptr<DiscreteFieldBlock> DiscreteField::createDiscreteFieldBlock(
    const string &name,
    const shared_ptr<DiscretizationBlock> &discretizationblock) {
  assert(discretizationblock->discretization()->manifold().get() ==
         field()->manifold().get());
  auto discretefieldblock =
      DiscreteFieldBlock::create(name, shared_from_this(), discretizationblock);
  checked_emplace(m_discretefieldblocks, discretefieldblock->name(),
                  discretefieldblock, "DiscreteField", "discretefieldblocks");
  assert(discretefieldblock->invariant());
  return discretefieldblock;
}

shared_ptr<DiscreteFieldBlock> DiscreteField::getDiscreteFieldBlock(
    const string &name,
    const shared_ptr<DiscretizationBlock> &discretizationblock) {
  auto loc = m_discretefieldblocks.find(name);
  if (loc != m_discretefieldblocks.end()) {
    const auto &discretefieldblock = loc->second;
    assert(discretefieldblock->discretizationblock() == discretizationblock);
    return discretefieldblock;
  }
  return createDiscreteFieldBlock(name, discretizationblock);
}

shared_ptr<DiscreteFieldBlock> DiscreteField::copyDiscreteFieldBlock(
    const shared_ptr<DiscreteFieldBlock> &discretefieldblock,
    bool copy_children) {
  auto discretizationblock2 = discretization()->copyDiscretizationBlock(
      discretefieldblock->discretizationblock());
  auto discretefieldblock2 =
      getDiscreteFieldBlock(discretefieldblock->name(), discretizationblock2);
  if (copy_children) {
    for (const auto &discretefieldblockcomponent_kv :
         discretefieldblock->discretefieldblockcomponents()) {
      const auto &discretefieldblockcomponent =
          discretefieldblockcomponent_kv.second;
      auto discretefieldblockcomponent2 =
          discretefieldblock2->copyDiscreteFieldBlockComponent(
              discretefieldblockcomponent, copy_children);
    }
  }
  return discretefieldblock2;
}

#ifdef SIMULATIONIO_HAVE_HDF5
shared_ptr<DiscreteFieldBlock>
DiscreteField::readDiscreteFieldBlock(const H5::H5Location &loc,
                                      const string &entry) {
  auto discretefieldblock =
      DiscreteFieldBlock::create(loc, entry, shared_from_this());
  checked_emplace(m_discretefieldblocks, discretefieldblock->name(),
                  discretefieldblock, "DiscreteField", "discretefieldblocks");
  assert(discretefieldblock->invariant());
  return discretefieldblock;
}
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
shared_ptr<DiscreteFieldBlock>
DiscreteField::readDiscreteFieldBlock(const shared_ptr<ASDF::reader_state> &rs,
                                      const YAML::Node &node) {
  auto discretefieldblock =
      DiscreteFieldBlock::create(rs, node, shared_from_this());
  checked_emplace(m_discretefieldblocks, discretefieldblock->name(),
                  discretefieldblock, "DiscreteField", "discretefieldblocks");
  assert(discretefieldblock->invariant());
  return discretefieldblock;
}
#endif

} // namespace SimulationIO
