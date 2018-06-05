#include "TensorType.hpp"

#include "TensorComponent.hpp"

#include "H5Helpers.hpp"

namespace SimulationIO {

bool TensorType::invariant() const {
  bool inv = Common::invariant() && bool(project()) &&
             project()->tensortypes().count(name()) &&
             project()->tensortypes().at(name()).get() == this &&
             dimension() >= 0 && rank() >= 0 &&
             int(tensorcomponents().size()) <= ipow(dimension(), rank());
  for (const auto &tc : tensorcomponents())
    inv &= !tc.first.empty() && bool(tc.second);
  return inv;
}

void TensorType::read(const H5::H5Location &loc, const string &entry,
                      const shared_ptr<Project> &project) {
  m_project = project;
  auto group = loc.openGroup(entry);
  assert(H5::readAttribute<string>(group, "type", project->enumtype) ==
         "TensorType");
  H5::readAttribute(group, "name", m_name);
  assert(H5::readGroupAttribute<string>(group, "project", "name") ==
         project->name());
  H5::readAttribute(group, "dimension", m_dimension);
  H5::readAttribute(group, "rank", m_rank);
  H5::readGroup(group, "tensorcomponents",
                [&](const H5::Group &group, const string &name) {
                  readTensorComponent(group, name);
                });
  // TODO: check storage_indices
}

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
void TensorType::read(const ASDF::reader_state &rs, const YAML::Node &node,
                      const shared_ptr<Project> &project) {
  assert(node.Tag() ==
         "tag:github.com/eschnett/SimulationIO/asdf-cxx/TensorType-1.0.0");
  m_name = node["name"].Scalar();
  m_project = project;
  m_dimension = node["dimension"].as<int>();
  m_rank = node["rank"].as<int>();
  for (const auto &kv : node["tensorcomponents"])
    readTensorComponent(rs, kv.second);
  // TODO: check storage_indices
}
#endif

void TensorType::merge(const shared_ptr<TensorType> &tensortype) {
  assert(project()->name() == tensortype->project()->name());
  assert(m_dimension == tensortype->dimension());
  assert(m_rank == tensortype->rank());
  for (const auto &iter : tensortype->tensorcomponents()) {
    const auto &tensorcomponent = iter.second;
    if (!m_tensorcomponents.count(tensorcomponent->name()))
      createTensorComponent(tensorcomponent->name(),
                            tensorcomponent->storage_index(),
                            tensorcomponent->indexvalues());
    m_tensorcomponents.at(tensorcomponent->name())->merge(tensorcomponent);
  }
  for (const auto &iter : tensortype->storage_indices()) {
    auto storage_index = iter.first;
    assert(m_storage_indices.at(storage_index)->name() ==
           tensortype->storage_indices().at(storage_index)->name());
  }
}

ostream &TensorType::output(ostream &os, int level) const {
  os << indent(level) << "TensorType " << quote(name())
     << ": dim=" << dimension() << " rank=" << rank() << "\n";
  for (const auto &tc : storage_indices())
    tc.second->output(os, level + 1);
  return os;
}

void TensorType::write(const H5::H5Location &loc,
                       const H5::H5Location &parent) const {
  assert(invariant());
  auto group = loc.createGroup(name());
  H5::createAttribute(group, "type", project()->enumtype, "TensorType");
  H5::createAttribute(group, "name", name());
  // H5::createHardLink(group, "project", parent, ".");
  H5::createHardLink(group, "..", parent, ".");
  H5::createSoftLink(group, "project", "..");
  H5::createAttribute(group, "dimension", dimension());
  H5::createAttribute(group, "rank", rank());
  H5::createGroup(group, "tensorcomponents", tensorcomponents());
  // TODO: write storage_indices
}

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
vector<string> TensorType::yaml_path() const {
  return concat(project()->yaml_path(), {"tensortypes", name()});
}

ASDF::writer &TensorType::write(ASDF::writer &w) const {
  auto aw = asdf_writer(w);
  aw.value("dimension", dimension());
  aw.value("rank", rank());
  aw.group("tensorcomponents", tensorcomponents());
  aw.alias_group("storage_indices", storage_indices());
  return w;
}
#endif

shared_ptr<TensorComponent>
TensorType::createTensorComponent(const string &name, int storage_index,
                                  const vector<int> &indexvalues) {
  auto tensorcomponent = TensorComponent::create(name, shared_from_this(),
                                                 storage_index, indexvalues);
  checked_emplace(m_tensorcomponents, tensorcomponent->name(), tensorcomponent,
                  "TensorType", "tensorcomponents");
  checked_emplace(m_storage_indices, tensorcomponent->storage_index(),
                  tensorcomponent, "TensorType", "storage_indices");
  assert(tensorcomponent->invariant());
  return tensorcomponent;
}

shared_ptr<TensorComponent>
TensorType::getTensorComponent(const string &name, int storage_index,
                               const vector<int> &indexvalues) {
  auto loc = m_tensorcomponents.find(name);
  if (loc != m_tensorcomponents.end()) {
    const auto &tensorcomponent = loc->second;
    assert(tensorcomponent->storage_index() == storage_index);
    assert(tensorcomponent->indexvalues() == indexvalues);
    return tensorcomponent;
  }
  return createTensorComponent(name, storage_index, indexvalues);
}

shared_ptr<TensorComponent> TensorType::copyTensorComponent(
    const shared_ptr<TensorComponent> &tensorcomponent, bool copy_children) {
  auto tensorcomponent2 = getTensorComponent(tensorcomponent->name(),
                                             tensorcomponent->storage_index(),
                                             tensorcomponent->indexvalues());
  return tensorcomponent2;
}

shared_ptr<TensorComponent>
TensorType::readTensorComponent(const H5::H5Location &loc,
                                const string &entry) {
  auto tensorcomponent =
      TensorComponent::create(loc, entry, shared_from_this());
  checked_emplace(m_tensorcomponents, tensorcomponent->name(), tensorcomponent,
                  "TensorType", "tensorcomponents");
  checked_emplace(m_storage_indices, tensorcomponent->storage_index(),
                  tensorcomponent, "TensorType", "storage_indices");
  assert(tensorcomponent->invariant());
  return tensorcomponent;
}

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
shared_ptr<TensorComponent>
TensorType::readTensorComponent(const ASDF::reader_state &rs,
                                const YAML::Node &node) {
  auto tensorcomponent = TensorComponent::create(rs, node, shared_from_this());
  checked_emplace(m_tensorcomponents, tensorcomponent->name(), tensorcomponent,
                  "TensorType", "tensorcomponents");
  checked_emplace(m_storage_indices, tensorcomponent->storage_index(),
                  tensorcomponent, "TensorType", "storage_indices");
  assert(tensorcomponent->invariant());
  return tensorcomponent;
}

shared_ptr<TensorComponent>
TensorType::getTensorComponent(const ASDF::reader_state &rs,
                               const YAML::Node &node) {
  auto ref = ASDF::reference(rs, node);
  auto doc_path = ref.get_split_target();
  const auto &doc = doc_path.first;
  const auto &path = doc_path.second;
  assert(doc.empty());
  assert(path.at(0) == project()->name());
  assert(path.at(1) == "tensortypes");
  assert(path.at(2) == name());
  assert(path.at(3) == "tensorcomponents");
  const auto &tensorcomponent_name = path.at(4);
  return tensorcomponents().at(tensorcomponent_name);
}
#endif

} // namespace SimulationIO
