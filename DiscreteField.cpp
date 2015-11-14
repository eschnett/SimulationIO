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
  configuration = field->project.lock()->configurations.at(
      H5::readGroupAttribute<string>(group, "configuration", "name"));
  assert(H5::readGroupAttribute<string>(
             group, string("configuration/discretefields/") + name, "name") ==
         name);
  discretization = field->manifold->discretizations.at(
      H5::readGroupAttribute<string>(group, "discretization", "name"));
  basis = field->tangentspace->bases.at(
      H5::readGroupAttribute<string>(group, "basis", "name"));
  H5::readGroup(group, "discretefieldblocks",
                [&](const H5::Group &group, const string &name) {
                  readDiscreteFieldBlock(group, name);
                });
  configuration->insert(name, shared_from_this());
  discretization->noinsert(shared_from_this());
  basis->noinsert(shared_from_this());
}

ostream &DiscreteField::output(ostream &os, int level) const {
  os << indent(level) << "DiscreteField " << quote(name) << ": Configuration "
     << quote(configuration->name) << " Field " << quote(field.lock()->name)
     << " Discretization " << quote(discretization->name) << " Basis "
     << quote(basis->name) << "\n";
  for (const auto &db : discretefieldblocks)
    db.second->output(os, level + 1);
  return os;
}

void DiscreteField::write(const H5::CommonFG &loc,
                          const H5::H5Location &parent) const {
  assert(invariant());
  auto group = loc.createGroup(name);
  H5::createAttribute(group, "type", field.lock()->project.lock()->enumtype,
                      "DiscreteField");
  H5::createAttribute(group, "name", name);
  H5::createHardLink(group, "field", parent, ".");
  H5::createHardLink(group, "configuration", parent,
                     string("project/configurations/") + configuration->name);
  H5::createHardLink(group, string("field/project/configurations/") +
                                configuration->name + "/discretefields",
                     name, group, ".");
  H5::createHardLink(group, "discretization", parent,
                     string("manifold/discretizations/") +
                         discretization->name);
  H5::createHardLink(group, "basis", parent,
                     string("tangentspace/bases/") + basis->name);
  createGroup(group, "discretefieldblocks", discretefieldblocks);
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
DiscreteField::readDiscreteFieldBlock(const H5::CommonFG &loc,
                                      const string &entry) {
  auto discretefieldblock =
      DiscreteFieldBlock::create(loc, entry, shared_from_this());
  checked_emplace(discretefieldblocks, discretefieldblock->name,
                  discretefieldblock);
  assert(discretefieldblock->invariant());
  return discretefieldblock;
}
}
