#include "TensorType.hpp"

#include "TensorComponent.hpp"

#include "H5Helpers.hpp"

namespace SimulationIO {

void TensorType::read(const H5::CommonFG &loc, const string &entry,
                      const shared_ptr<Project> &project) {
  this->project = project;
  auto group = loc.openGroup(entry);
  assert(H5::readAttribute<string>(group, "type", project->enumtype) ==
         "TensorType");
  H5::readAttribute(group, "name", m_name);
  assert(H5::readGroupAttribute<string>(group, "project", "name") ==
         project->name());
  H5::readAttribute(group, "dimension", dimension);
  H5::readAttribute(group, "rank", rank);
  H5::readGroup(group, "tensorcomponents",
                [&](const H5::Group &group, const string &name) {
                  readTensorComponent(group, name);
                });
  // TODO: check storage_indices
}

ostream &TensorType::output(ostream &os, int level) const {
  os << indent(level) << "TensorType " << quote(name()) << ": dim=" << dimension
     << " rank=" << rank << "\n";
  for (const auto &tc : storage_indices)
    tc.second->output(os, level + 1);
  return os;
}

void TensorType::write(const H5::CommonFG &loc,
                       const H5::H5Location &parent) const {
  assert(invariant());
  auto group = loc.createGroup(name());
  H5::createAttribute(group, "type", project.lock()->enumtype, "TensorType");
  H5::createAttribute(group, "name", name());
  // H5::createHardLink(group, "project", parent, ".");
  H5::createHardLink(group, "..", parent, ".");
  H5::createSoftLink(group, "project", "..");
  H5::createAttribute(group, "dimension", dimension);
  H5::createAttribute(group, "rank", rank);
  H5::createGroup(group, "tensorcomponents", tensorcomponents);
  // TODO: write storage_indices
}

shared_ptr<TensorComponent>
TensorType::createTensorComponent(const string &name, int stored_component,
                                  const vector<int> &indexvalues) {
  auto tensorcomponent = TensorComponent::create(name, shared_from_this(),
                                                 stored_component, indexvalues);
  checked_emplace(tensorcomponents, tensorcomponent->name(), tensorcomponent);
  checked_emplace(storage_indices, tensorcomponent->storage_index,
                  tensorcomponent);
  assert(tensorcomponent->invariant());
  return tensorcomponent;
}

shared_ptr<TensorComponent>
TensorType::readTensorComponent(const H5::CommonFG &loc, const string &entry) {
  auto tensorcomponent =
      TensorComponent::create(loc, entry, shared_from_this());
  checked_emplace(tensorcomponents, tensorcomponent->name(), tensorcomponent);
  checked_emplace(storage_indices, tensorcomponent->storage_index,
                  tensorcomponent);
  assert(tensorcomponent->invariant());
  return tensorcomponent;
}
}
