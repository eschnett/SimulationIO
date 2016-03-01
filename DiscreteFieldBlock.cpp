#include "DiscreteFieldBlock.hpp"

#include "DiscreteFieldBlockComponent.hpp"
#include "DiscretizationBlock.hpp"

#include "H5Helpers.hpp"

namespace SimulationIO {

void DiscreteFieldBlock::read(const H5::CommonFG &loc, const string &entry,
                              const shared_ptr<DiscreteField> &discretefield) {
  this->discretefield = discretefield;
  auto group = loc.openGroup(entry);
  assert(H5::readAttribute<string>(
             group, "type",
             discretefield->field.lock()->project.lock()->enumtype) ==
         "DiscreteFieldBlock");
  H5::readAttribute(group, "name", name);
  assert(H5::readGroupAttribute<string>(group, "discretefield", "name") ==
         discretefield->name);
  // TODO: Read and interpret objects (shallowly) instead of naively only
  // looking at their names
  discretizationblock = discretefield->discretization->discretizationblocks.at(
      H5::readGroupAttribute<string>(group, "discretizationblock", "name"));
  H5::readGroup(group, "discretefieldblockcomponents",
                [&](const H5::Group &group, const string &name) {
                  readDiscreteFieldBlockComponent(group, name);
                });
  discretizationblock->noinsert(shared_from_this());
#warning "TODO: check storage_indices"
}

ostream &DiscreteFieldBlock::output(ostream &os, int level) const {
  os << indent(level) << "DiscreteFieldBlock " << quote(name)
     << ": DiscreteField " << quote(discretefield.lock()->name)
     << " DiscretizationBlock " << quote(discretizationblock->name) << "\n";
  for (const auto &dfbd : discretefieldblockcomponents)
    dfbd.second->output(os, level + 1);
  return os;
}

void DiscreteFieldBlock::write(const H5::CommonFG &loc,
                               const H5::H5Location &parent) const {
  assert(invariant());
  auto group = loc.createGroup(name);
  H5::createAttribute(
      group, "type",
      discretefield.lock()->field.lock()->project.lock()->enumtype,
      "DiscreteFieldBlock");
  H5::createAttribute(group, "name", name);
  H5::createHardLink(group, "discretefield", parent, ".");
  H5::createHardLink(group, "discretizationblock", parent,
                     string("discretization/discretizationblocks/") +
                         discretizationblock->name);
  H5::createGroup(group, "discretefieldblockcomponents",
                  discretefieldblockcomponents);
#warning "TODO: write storage_indices"
}

shared_ptr<DiscreteFieldBlockComponent>
DiscreteFieldBlock::createDiscreteFieldBlockComponent(
    const string &name, const shared_ptr<TensorComponent> &tensorcomponent) {
  auto discretefieldblockcomponent = DiscreteFieldBlockComponent::create(
      name, shared_from_this(), tensorcomponent);
  checked_emplace(discretefieldblockcomponents,
                  discretefieldblockcomponent->name,
                  discretefieldblockcomponent);
  checked_emplace(storage_indices,
                  discretefieldblockcomponent->tensorcomponent->storage_index,
                  discretefieldblockcomponent);
  assert(discretefieldblockcomponent->invariant());
  return discretefieldblockcomponent;
}

shared_ptr<DiscreteFieldBlockComponent>
DiscreteFieldBlock::readDiscreteFieldBlockComponent(const H5::CommonFG &loc,
                                                    const string &entry) {
  auto discretefieldblockcomponent =
      DiscreteFieldBlockComponent::create(loc, entry, shared_from_this());
  checked_emplace(discretefieldblockcomponents,
                  discretefieldblockcomponent->name,
                  discretefieldblockcomponent);
  checked_emplace(storage_indices,
                  discretefieldblockcomponent->tensorcomponent->storage_index,
                  discretefieldblockcomponent);
  assert(discretefieldblockcomponent->invariant());
  return discretefieldblockcomponent;
}
}
