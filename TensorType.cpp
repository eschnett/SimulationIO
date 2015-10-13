#include "TensorType.hpp"

#include "TensorComponent.hpp"

#include "H5Helpers.hpp"

namespace SimulationIO {

TensorType::TensorType(const H5::CommonFG &loc, const string &entry,
                       Project *project)
    : project(project) {
  auto group = loc.openGroup(entry);
  string type;
  H5::readAttribute(group, "type", type);
  assert(type == "TensorType");
  H5::readAttribute(group, "name", name);
  // TODO: check link "project"
  H5::readAttribute(group, "dimension", dimension);
  H5::readAttribute(group, "rank", rank);
  H5::readGroup(group, "tensorcomponents",
                [&](const string &name, const H5::Group &group) {
                  createTensorComponent(group, name);
                },
                tensorcomponents);
  //  TODO: check "storage_indices"
}

ostream &TensorType::output(ostream &os, int level) const {
  os << indent(level) << "TensorType \"" << name << "\": dim=" << dimension
     << " rank=" << rank << "\n";
  for (const auto &tc : storage_indices)
    tc.second->output(os, level + 1);
  return os;
}

void TensorType::write(const H5::CommonFG &loc,
                       const H5::H5Location &parent) const {
  auto group = loc.createGroup(name);
  H5::createAttribute(group, "type", "TensorType");
  H5::createAttribute(group, "name", name);
  H5::createAttribute(group, "project", parent, ".");
  H5::createAttribute(group, "dimension", dimension);
  H5::createAttribute(group, "rank", rank);
  H5::createGroup(group, "tensorcomponents", tensorcomponents);
#warning "TODO: output storage_indices"
}

TensorComponent *
TensorType::createTensorComponent(const string &name, int stored_component,
                                  const vector<int> &indexvalues) {
  auto tensorcomponent =
      new TensorComponent(name, this, stored_component, indexvalues);
  checked_emplace(tensorcomponents, tensorcomponent->name, tensorcomponent);
  checked_emplace(storage_indices, tensorcomponent->storage_index,
                  tensorcomponent);
  return tensorcomponent;
}

TensorComponent *TensorType::createTensorComponent(const H5::CommonFG &loc,
                                                   const string &entry) {
  auto tensorcomponent = new TensorComponent(loc, entry, this);
  checked_emplace(tensorcomponents, tensorcomponent->name, tensorcomponent);
  checked_emplace(storage_indices, tensorcomponent->storage_index,
                  tensorcomponent);
  return tensorcomponent;
}
}
