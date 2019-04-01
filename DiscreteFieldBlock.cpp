#include "DiscreteFieldBlock.hpp"

#include "DiscreteFieldBlockComponent.hpp"
#include "DiscretizationBlock.hpp"

#ifdef SIMULATIONIO_HAVE_HDF5
#include "H5Helpers.hpp"
#endif

namespace SimulationIO {

bool DiscreteFieldBlock::invariant() const {
  bool inv = Common::invariant() && bool(discretefield()) &&
             discretefield()->discretefieldblocks().count(name()) &&
             discretefield()->discretefieldblocks().at(name()).get() == this &&
             bool(discretizationblock()) &&
             discretizationblock()->discretefieldblocks().nobacklink() &&
             discretefieldblockcomponents().size() == storage_indices().size();
  return inv;
}

#ifdef SIMULATIONIO_HAVE_HDF5
void DiscreteFieldBlock::read(const H5::H5Location &loc, const string &entry,
                              const shared_ptr<DiscreteField> &discretefield) {
  m_discretefield = discretefield;
  auto group = loc.openGroup(entry);
  assert(H5::readAttribute<string>(
             group, "type", discretefield->field()->project()->enumtype) ==
         "DiscreteFieldBlock");
  H5::readAttribute(group, "name", m_name);
  assert(H5::readGroupAttribute<string>(group, "discretefield", "name") ==
         discretefield->name());
  // TODO: Read and interpret objects (shallowly) instead of naively only
  // looking at their names
  m_discretizationblock =
      discretefield->discretization()->discretizationblocks().at(
          H5::readGroupAttribute<string>(group, "discretizationblock", "name"));
  H5::readGroup(group, "discretefieldblockcomponents",
                [&](const H5::Group &group, const string &name) {
                  readDiscreteFieldBlockComponent(group, name);
                });
  m_discretizationblock->noinsert(shared_from_this());
  // TODO: check storage_indices
}
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
void DiscreteFieldBlock::read(const shared_ptr<ASDF::reader_state> &rs,
                              const YAML::Node &node,
                              const shared_ptr<DiscreteField> &discretefield) {
  assert(
      node.Tag() ==
      "tag:github.com/eschnett/SimulationIO/asdf-cxx/DiscreteFieldBlock-1.0.0");
  m_name = node["name"].Scalar();
  m_discretefield = discretefield;

  m_discretizationblock =
      discretefield->discretization()->getDiscretizationBlock(
          rs, node["discretizationblock"]);
  for (const auto &kv : node["discretefieldblockcomponents"])
    readDiscreteFieldBlockComponent(rs, kv.second);
  m_discretizationblock->noinsert(shared_from_this());
  // TODO: check storage_indices
}
#endif

void DiscreteFieldBlock::merge(
    const shared_ptr<DiscreteFieldBlock> &discretefieldblock) {
  assert(discretefield()->name() ==
         discretefieldblock->discretefield()->name());
  assert(m_discretizationblock->name() ==
         discretefieldblock->discretizationblock()->name());
  for (const auto &iter : discretefieldblock->discretefieldblockcomponents()) {
    const auto &discretefieldblockcomponent = iter.second;
    if (!m_discretefieldblockcomponents.count(
            discretefieldblockcomponent->name()))
      createDiscreteFieldBlockComponent(
          discretefieldblockcomponent->name(),
          discretefield()
              ->field()
              ->project()
              ->tensortypes()
              .at(discretefieldblockcomponent->tensorcomponent()
                      ->tensortype()
                      ->name())
              ->tensorcomponents()
              .at(discretefieldblockcomponent->tensorcomponent()->name()));
    m_discretefieldblockcomponents.at(discretefieldblockcomponent->name())
        ->merge(discretefieldblockcomponent);
  }
  for (const auto &iter : discretefieldblock->storage_indices()) {
    auto storage_index = iter.first;
    assert(m_storage_indices.at(storage_index)->name() ==
           discretefieldblock->storage_indices().at(storage_index)->name());
  }
}

ostream &DiscreteFieldBlock::output(ostream &os, int level) const {
  os << indent(level) << "DiscreteFieldBlock " << quote(name())
     << ": DiscreteField " << quote(discretefield()->name())
     << " DiscretizationBlock " << quote(discretizationblock()->name()) << "\n";
  for (const auto &dfbd : discretefieldblockcomponents())
    dfbd.second->output(os, level + 1);
  return os;
}

#ifdef SIMULATIONIO_HAVE_HDF5
void DiscreteFieldBlock::write(const H5::H5Location &loc,
                               const H5::H5Location &parent) const {
  assert(invariant());
  auto group = loc.createGroup(name());
  H5::createAttribute(group, "type",
                      discretefield()->field()->project()->enumtype,
                      "DiscreteFieldBlock");
  H5::createAttribute(group, "name", name());
  // H5::createHardLink(group, "discretefield", parent, ".");
  H5::createHardLink(group, "..", parent, ".");
  H5::createSoftLink(group, "discretefield", "..");
  // H5::createHardLink(group, "discretizationblock", parent,
  //                    "discretization/discretizationblocks/" +
  //                        discretizationblock->name());
  H5::createSoftLink(group, "discretizationblock",
                     "../discretization/discretizationblocks/" +
                         discretizationblock()->name());
  H5::createGroup(group, "discretefieldblockcomponents",
                  discretefieldblockcomponents());
  // TODO: write storage_indices
}
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
vector<string> DiscreteFieldBlock::yaml_path() const {
  return concat(discretefield()->yaml_path(), {"discretefieldblocks", name()});
}

ASDF::writer &DiscreteFieldBlock::write(ASDF::writer &w) const {
  auto aw = asdf_writer(w);
  aw.alias("discretizationblock", *discretizationblock());
  aw.group("discretefieldblockcomponents", discretefieldblockcomponents());
  aw.alias_group("storage_indices", storage_indices());
  return w;
}
#endif

#ifdef SIMULATIONIO_HAVE_TILEDB
vector<string> DiscreteFieldBlock::tiledb_path() const {
  return concat(discretefield()->tiledb_path(),
                {"discretefieldblocks", name()});
}

void DiscreteFieldBlock::write(const tiledb::Context &ctx,
                               const string &loc) const {
  assert(invariant());
  const tiledb_writer w(*this, ctx, loc);
  w.add_symlink(concat(tiledb_path(), {"discretizationblock"}),
                discretizationblock()->tiledb_path());
  w.add_group("discretefieldblockcomponents", discretefieldblockcomponents());
  w.create_group("storage_indices");
  for (const auto &kv : storage_indices()) {
    const auto &idx = kv.first;
    const auto &storage_index = kv.second;
    string idx_string = to_string(idx);
    w.add_symlink(concat(tiledb_path(), {"storage_indices", idx_string}),
                  storage_index->tiledb_path());
  }
}
#endif

shared_ptr<DiscreteFieldBlockComponent>
DiscreteFieldBlock::createDiscreteFieldBlockComponent(
    const string &name, const shared_ptr<TensorComponent> &tensorcomponent) {
  assert(tensorcomponent->tensortype().get() ==
         discretefield()->field()->tensortype().get());
  auto discretefieldblockcomponent = DiscreteFieldBlockComponent::create(
      name, shared_from_this(), tensorcomponent);
  checked_emplace(m_discretefieldblockcomponents,
                  discretefieldblockcomponent->name(),
                  discretefieldblockcomponent, "DiscreteFieldBlock",
                  "discretefieldblockcomponents");
  checked_emplace(
      m_storage_indices,
      discretefieldblockcomponent->tensorcomponent()->storage_index(),
      discretefieldblockcomponent, "DiscreteFieldBlock", "storage_indices");
  assert(discretefieldblockcomponent->invariant());
  return discretefieldblockcomponent;
}

shared_ptr<DiscreteFieldBlockComponent>
DiscreteFieldBlock::getDiscreteFieldBlockComponent(
    const string &name, const shared_ptr<TensorComponent> &tensorcomponent) {
  auto loc = m_discretefieldblockcomponents.find(name);
  if (loc != m_discretefieldblockcomponents.end()) {
    const auto &discretefieldblockcomponent = loc->second;
    assert(discretefieldblockcomponent->tensorcomponent() == tensorcomponent);
    return discretefieldblockcomponent;
  }
  return createDiscreteFieldBlockComponent(name, tensorcomponent);
}

shared_ptr<DiscreteFieldBlockComponent>
DiscreteFieldBlock::copyDiscreteFieldBlockComponent(
    const shared_ptr<DiscreteFieldBlockComponent> &discretefieldblockcomponent,
    bool copy_children) {
  auto tensorcomponent2 =
      discretefield()->field()->tensortype()->copyTensorComponent(
          discretefieldblockcomponent->tensorcomponent());
  auto discretefieldblockcomponent2 = getDiscreteFieldBlockComponent(
      discretefieldblockcomponent->name(), tensorcomponent2);
  if (copy_children) {
    auto datablock = discretefieldblockcomponent->datablock();
    if (datablock) {
#ifdef SIMULATIONIO_HAVE_HDF5
      auto copyobj = discretefieldblockcomponent->copyobj();
      if (copyobj) {
        // Copy object only if it does not already exist
        auto copyobj2 = discretefieldblockcomponent2->copyobj();
        if (!copyobj2)
          copyobj2 = discretefieldblockcomponent2->createCopyObj(
              WriteOptions(), copyobj->group(), copyobj->name());
      }
#endif
      auto datarange = discretefieldblockcomponent->datarange();
      if (datarange) {
        // Copy data range only if it does not already exist
        auto datarange2 = discretefieldblockcomponent2->datarange();
        if (!datarange2)
          datarange2 = discretefieldblockcomponent2->createDataRange(
              WriteOptions(), datarange->origin(), datarange->delta());
      }
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
#warning "TODO: handle ASDF types"
      assert(0);
#endif
    }
  }
  return discretefieldblockcomponent2;
}

#ifdef SIMULATIONIO_HAVE_HDF5
shared_ptr<DiscreteFieldBlockComponent>
DiscreteFieldBlock::readDiscreteFieldBlockComponent(const H5::H5Location &loc,
                                                    const string &entry) {
  auto discretefieldblockcomponent =
      DiscreteFieldBlockComponent::create(loc, entry, shared_from_this());
  checked_emplace(m_discretefieldblockcomponents,
                  discretefieldblockcomponent->name(),
                  discretefieldblockcomponent, "DiscreteFieldBlock",
                  "discretefieldblockcomponents");
  checked_emplace(
      m_storage_indices,
      discretefieldblockcomponent->tensorcomponent()->storage_index(),
      discretefieldblockcomponent, "DiscreteFieldBlock", "storage_indices");
  assert(discretefieldblockcomponent->invariant());
  return discretefieldblockcomponent;
}
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
shared_ptr<DiscreteFieldBlockComponent>
DiscreteFieldBlock::readDiscreteFieldBlockComponent(
    const shared_ptr<ASDF::reader_state> &rs, const YAML::Node &node) {
  auto discretefieldblockcomponent =
      DiscreteFieldBlockComponent::create(rs, node, shared_from_this());
  checked_emplace(m_discretefieldblockcomponents,
                  discretefieldblockcomponent->name(),
                  discretefieldblockcomponent, "DiscreteFieldBlock",
                  "discretefieldblockcomponents");
  checked_emplace(
      m_storage_indices,
      discretefieldblockcomponent->tensorcomponent()->storage_index(),
      discretefieldblockcomponent, "DiscreteFieldBlock", "storage_indices");
  assert(discretefieldblockcomponent->invariant());
  return discretefieldblockcomponent;
}
#endif

} // namespace SimulationIO
