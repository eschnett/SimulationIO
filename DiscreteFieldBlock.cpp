#include "DiscreteFieldBlock.hpp"

#include "DiscreteFieldBlockData.hpp"
#include "DiscretizationBlock.hpp"

#include "H5Helpers.hpp"

namespace SimulationIO {

DiscreteFieldBlock::DiscreteFieldBlock(const H5::CommonFG &loc,
                                       const string &entry,
                                       DiscreteField *discretefield)
    : discretefield(discretefield) {
  auto group = loc.openGroup(entry);
  assert(H5::readAttribute<string>(group, "type",
                                   discretefield->field->project->enumtype) ==
         "DiscreteFieldBlock");
  H5::readAttribute(group, "name", name);
  assert(H5::readGroupAttribute<string>(group, "discretefield", "name") ==
         discretefield->name);
  // TODO: Read and interpret objects (shallowly) instead of naively only
  // looking at their names
  discretizationblock = discretefield->discretization->discretizationblocks.at(
      H5::readGroupAttribute<string>(group, "discretizationblock", "name"));
  H5::readGroup(group, "discretefieldblockdata",
                [&](const H5::Group &group, const string &name) {
                  createDiscreteFieldBlockData(group, name);
                });
  discretizationblock->noinsert(this);
}

ostream &DiscreteFieldBlock::output(ostream &os, int level) const {
  os << indent(level) << "DiscreteFieldBlock \"" << name
     << "\": discretefield=\"" << discretefield->name
     << "\" discretizationblock=\"" << discretizationblock->name << "\"\n";
  for (const auto &dfbd : discretefieldblockdata)
    dfbd.second->output(os, level + 1);
  return os;
}

void DiscreteFieldBlock::write(const H5::CommonFG &loc,
                               const H5::H5Location &parent) const {
  auto group = loc.createGroup(name);
  H5::createAttribute(group, "type", discretefield->field->project->enumtype,
                      "DiscreteFieldBlock");
  H5::createAttribute(group, "name", name);
  H5::createHardLink(group, "discretefield", parent, ".");
  H5::createHardLink(group, "discretizationblock", parent,
                     string("discretization/discretizationblocks/") +
                         discretizationblock->name);
  H5::createGroup(group, "discretefieldblockdata", discretefieldblockdata);
}

DiscreteFieldBlockData *DiscreteFieldBlock::createDiscreteFieldBlockData(
    const string &name, TensorComponent *tensorcomponent) {
  auto discretefieldblockdata =
      new DiscreteFieldBlockData(name, this, tensorcomponent);
  checked_emplace(this->discretefieldblockdata, discretefieldblockdata->name,
                  discretefieldblockdata);
  assert(discretefieldblockdata->invariant());
  return discretefieldblockdata;
}

DiscreteFieldBlockData *
DiscreteFieldBlock::createDiscreteFieldBlockData(const H5::CommonFG &loc,
                                                 const string &entry) {
  auto discretefieldblockdata = new DiscreteFieldBlockData(loc, entry, this);
  checked_emplace(this->discretefieldblockdata, discretefieldblockdata->name,
                  discretefieldblockdata);
  assert(discretefieldblockdata->invariant());
  return discretefieldblockdata;
}
}
