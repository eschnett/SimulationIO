#include "DiscreteFieldBlock.hpp"

#include "DiscreteFieldBlockComponent.hpp"
#include "DiscretizationBlock.hpp"

#include "H5Helpers.hpp"

namespace SimulationIO {

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
      auto copyobj = discretefieldblockcomponent->copyobj();
      auto datarange = discretefieldblockcomponent->datarange();
      if (copyobj) {
        // Copy object only if it does not already exist
        auto copyobj2 = discretefieldblockcomponent2->copyobj();
        if (!copyobj2)
          copyobj2 = discretefieldblockcomponent2->createCopyObj(
              copyobj->group(), copyobj->name());
      } else if (datarange) {
        // Copy data range only if it does not already exist
        auto datarange2 = discretefieldblockcomponent2->datarange();
        if (!datarange2)
          datarange2 = discretefieldblockcomponent2->createDataRange(
              datarange->origin(), datarange->delta());
      }
    }
  }
  return discretefieldblockcomponent2;
}

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
} // namespace SimulationIO
