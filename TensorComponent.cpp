#include "TensorComponent.hpp"

#include "H5Helpers.hpp"

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
} // namespace SimulationIO
