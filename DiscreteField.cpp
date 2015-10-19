#include "DiscreteField.hpp"

#include "DiscreteFieldBlock.hpp"

#include "H5Helpers.hpp"

namespace SimulationIO {

void DiscreteField::read(const H5::CommonFG &loc, const string &entry,
                         const shared_ptr<Field> &field) {
  this->field = field;
  auto group = loc.openGroup(entry);
  assert(H5::readAttribute<string>(group, "type",
                                   field->project.lock()->enumtype) ==
         "DiscreteField");
  H5::readAttribute(group, "name", name);
  assert(H5::readGroupAttribute<string>(group, "field", "name") == field->name);
  // TODO: Read and interpret objects (shallowly) instead of naively only
  // looking at their names
  discretization = field->manifold->discretizations.at(
      H5::readGroupAttribute<string>(group, "discretization", "name"));
  basis = field->tangentspace->bases.at(
      H5::readGroupAttribute<string>(group, "basis", "name"));
  H5::readGroup(group, "discretefieldblocks",
                [&](const H5::Group &group, const string &name) {
                  createDiscreteFieldBlock(group, name);
                });
  discretization->noinsert(shared_from_this());
  basis->noinsert(shared_from_this());
}

ostream &DiscreteField::output(ostream &os, int level) const {
  os << indent(level) << "DiscreteField \"" << name << "\": Field \""
     << field.lock()->name << "\" Discretization \"" << discretization->name
     << "\" Basis \"" << basis->name << "\"\n";
  for (const auto &db : discretefieldblocks)
    db.second->output(os, level + 1);
  return os;
}

void DiscreteField::write(const H5::CommonFG &loc,
                          const H5::H5Location &parent) const {
  auto group = loc.createGroup(name);
  H5::createAttribute(group, "type", field.lock()->project.lock()->enumtype,
                      "DiscreteField");
  H5::createAttribute(group, "name", name);
  H5::createHardLink(group, "field", parent, ".");
  H5::createHardLink(group, "discretization", parent,
                     string("manifold/discretizations/") +
                         discretization->name);
  H5::createHardLink(group, "basis", parent,
                     string("tangentspace/bases/") + basis->name);
  H5::createGroup(group, "discretefieldblocks", discretefieldblocks);
}

shared_ptr<DiscreteFieldBlock> DiscreteField::createDiscreteFieldBlock(
    const string &name,
    const shared_ptr<DiscretizationBlock> &discretizationblock) {
  auto discretefieldblock =
      DiscreteFieldBlock::create(name, shared_from_this(), discretizationblock);
  checked_emplace(discretefieldblocks, discretefieldblock->name,
                  discretefieldblock);
  assert(discretefieldblock->invariant());
  return discretefieldblock;
}

shared_ptr<DiscreteFieldBlock>
DiscreteField::createDiscreteFieldBlock(const H5::CommonFG &loc,
                                        const string &entry) {
  auto discretefieldblock =
      DiscreteFieldBlock::create(loc, entry, shared_from_this());
  checked_emplace(discretefieldblocks, discretefieldblock->name,
                  discretefieldblock);
  assert(discretefieldblock->invariant());
  return discretefieldblock;
}
}
