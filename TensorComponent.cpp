#include "TensorComponent.hpp"

#ifdef SIMULATIONIO_HAVE_HDF5
#include "H5Helpers.hpp"
#endif

namespace SimulationIO {

bool TensorComponent::invariant() const {
  bool inv =
      Common::invariant() && bool(tensortype()) &&
      tensortype()->tensorcomponents().count(name()) &&
      tensortype()->tensorcomponents().at(name()).get() == this &&
      storage_index() >= 0 &&
      storage_index() < ipow(tensortype()->dimension(), tensortype()->rank()) &&
      tensortype()->storage_indices().count(storage_index()) &&
      tensortype()->storage_indices().at(storage_index()).get() == this &&
      int(indexvalues().size()) == tensortype()->rank();
  for (int i = 0; i < int(indexvalues().size()); ++i)
    inv &= indexvalues().at(i) >= 0 &&
           indexvalues().at(i) < tensortype()->dimension();
  // Ensure all tensor components are distinct
  for (const auto &tc : tensortype()->tensorcomponents()) {
    const auto &other = tc.second;
    if (other.get() == this)
      continue;
    bool samesize = other->indexvalues().size() == indexvalues().size();
    inv &= samesize;
    if (samesize) {
      bool isequal = true;
      for (int i = 0; i < int(indexvalues().size()); ++i)
        isequal &= other->indexvalues().at(i) == indexvalues().at(i);
      inv &= !isequal;
    }
  }
  return inv;
}

#ifdef SIMULATIONIO_HAVE_HDF5
void TensorComponent::read(const H5::H5Location &loc, const string &entry,
                           const shared_ptr<TensorType> &tensortype) {
  m_tensortype = tensortype;
  auto group = loc.openGroup(entry);
  assert(H5::readAttribute<string>(group, "type",
                                   tensortype->project()->enumtype) ==
         "TensorComponent");
  H5::readAttribute(group, "name", m_name);
  assert(H5::readGroupAttribute<string>(group, "tensortype", "name") ==
         tensortype->name());
  H5::readAttribute(group, "storage_index", m_storage_index);
  H5::readAttribute(group, "indexvalues", m_indexvalues);
}
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
void TensorComponent::read(const shared_ptr<ASDF::reader_state> &rs,
                           const YAML::Node &node,
                           const shared_ptr<TensorType> &tensortype) {
  assert(node.Tag() ==
         "tag:github.com/eschnett/SimulationIO/asdf-cxx/TensorComponent-1.0.0");
  m_name = node["name"].Scalar();
  m_tensortype = tensortype;
  m_storage_index = node["storage_index"].as<int>();
  m_indexvalues = node["indexvalues"].as<vector<int>>();
}
#endif

void TensorComponent::merge(
    const shared_ptr<TensorComponent> &tensorcomponent) {
  assert(tensortype()->name() == tensorcomponent->tensortype()->name());
  assert(m_storage_index == tensorcomponent->storage_index());
  assert(m_indexvalues == tensorcomponent->indexvalues());
}

ostream &TensorComponent::output(ostream &os, int level) const {
  using namespace Output;
  os << indent(level) << "TensorComponent " << quote(name()) << ": TensorType "
     << quote(tensortype()->name()) << " storage_index=" << storage_index()
     << " indexvalues=" << indexvalues() << "\n";
  return os;
}

#ifdef SIMULATIONIO_HAVE_HDF5
void TensorComponent::write(const H5::H5Location &loc,
                            const H5::H5Location &parent) const {
  assert(invariant());
  auto group = loc.createGroup(name());
  H5::createAttribute(group, "type", tensortype()->project()->enumtype,
                      "TensorComponent");
  H5::createAttribute(group, "name", name());
  // H5::createHardLink(group, "tensortype", parent, ".");
  H5::createHardLink(group, "..", parent, ".");
  H5::createSoftLink(group, "tensortype", "..");
  H5::createAttribute(group, "storage_index", storage_index());
  H5::createAttribute(group, "indexvalues", indexvalues());
}
#endif

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
vector<string> TensorComponent::yaml_path() const {
  return concat(tensortype()->yaml_path(), {"tensorcomponents", name()});
}

ASDF::writer &TensorComponent::write(ASDF::writer &w) const {
  auto aw = asdf_writer(w);
  aw.value("storage_index", storage_index());
  aw.short_sequence("indexvalues", indexvalues());
  return w;
}
#endif

#ifdef SIMULATIONIO_HAVE_SILO
void TensorComponent::write(DBfile *const file, const string &loc) const {
  assert(invariant());
  write_attribute(file, loc, "storage_index", storage_index());
  write_attribute(file, loc, "indexvalues", indexvalues());
}
#endif

#ifdef SIMULATIONIO_HAVE_TILEDB
vector<string> TensorComponent::tiledb_path() const {
  return concat(tensortype()->tiledb_path(), {"tensorcomponents", name()});
}

void TensorComponent::write(const tiledb::Context &ctx,
                            const string &loc) const {
  assert(invariant());
  const tiledb_writer w(*this, ctx, loc);
  w.add_attribute("storage_index", storage_index());
  w.add_attribute("indexvalues", indexvalues());
}
#endif

} // namespace SimulationIO
