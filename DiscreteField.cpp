#include "DiscreteField.hpp"

#include "DiscreteFieldBlock.hpp"

#include "H5Helpers.hpp"

namespace SimulationIO {

void DiscreteField::read(const H5::CommonFG &loc, const string &entry,
                         const shared_ptr<Field> &field) {
  m_field = field;
  auto group = loc.openGroup(entry);
  assert(H5::readAttribute<string>(group, "type", field->project()->enumtype) ==
         "DiscreteField");
  H5::readAttribute(group, "name", m_name);
  assert(H5::readGroupAttribute<string>(group, "field", "name") ==
         field->name());
  // TODO: Read and interpret objects (shallowly) instead of naively only
  // looking at their names
  m_configuration = field->project()->configurations().at(
      H5::readGroupAttribute<string>(group, "configuration", "name"));
  assert(H5::readGroupAttribute<string>(
             group, "configuration/discretefields/" + name(), "name") ==
         name());
  m_discretization = field->manifold()->discretizations().at(
      H5::readGroupAttribute<string>(group, "discretization", "name"));
  m_basis = field->tangentspace()->bases().at(
      H5::readGroupAttribute<string>(group, "basis", "name"));
  H5::readGroup(group, "discretefieldblocks",
                [&](const H5::Group &group, const string &name) {
                  readDiscreteFieldBlock(group, name);
                });
  m_configuration->insert(name(), shared_from_this());
  m_discretization->noinsert(shared_from_this());
  m_basis->noinsert(shared_from_this());
}

void DiscreteField::merge(const shared_ptr<DiscreteField> &discretefield) {
  assert(field()->name() == discretefield->field()->name());
  assert(m_configuration->name() == discretefield->configuration()->name());
  assert(m_discretization->name() == discretefield->discretization()->name());
  assert(m_basis->name() == discretefield->basis()->name());
  for (const auto &iter : discretefield->discretefieldblocks()) {
    const auto &discretefieldblock = iter.second;
    if (!m_discretefieldblocks.count(discretefieldblock->name()))
      createDiscreteFieldBlock(
          discretefieldblock->name(),
          m_discretization->discretizationblocks().at(
              discretefieldblock->discretizationblock()->name()));
    m_discretefieldblocks.at(discretefieldblock->name())
        ->merge(discretefieldblock);
  }
}

ostream &DiscreteField::output(ostream &os, int level) const {
  os << indent(level) << "DiscreteField " << quote(name()) << ": Configuration "
     << quote(configuration()->name()) << " Field " << quote(field()->name())
     << " Discretization " << quote(discretization()->name()) << " Basis "
     << quote(basis()->name()) << "\n";
  for (const auto &db : discretefieldblocks())
    db.second->output(os, level + 1);
  return os;
}

void DiscreteField::write(const H5::CommonFG &loc,
                          const H5::H5Location &parent) const {
  assert(invariant());
  auto group = loc.createGroup(name());
  H5::createAttribute(group, "type", field()->project()->enumtype,
                      "DiscreteField");
  H5::createAttribute(group, "name", name());
  // H5::createHardLink(group, "field", parent, ".");
  H5::createHardLink(group, "..", parent, ".");
  H5::createSoftLink(group, "field", "..");
  // H5::createHardLink(group, "configuration", parent,
  //                    "project/configurations/" +
  //                    configuration->name());
  H5::createSoftLink(group, "configuration",
                     "../project/configurations/" + configuration()->name());
  H5::createHardLink(group, "field/project/configurations/" +
                                configuration()->name() + "/discretefields",
                     name(), group, ".");
  // H5::createHardLink(group, "discretization", parent,
  //                    "manifold/discretizations/" +
  //                        discretization->name());
  H5::createSoftLink(group, "discretization",
                     "../manifold/discretizations/" + discretization()->name());
  // H5::createHardLink(group, "basis", parent,
  //                    "tangentspace/bases/" + basis->name());
  H5::createSoftLink(group, "basis",
                     "../tangentspace/bases/" + basis()->name());
  createGroup(group, "discretefieldblocks", discretefieldblocks());
}

shared_ptr<DiscreteFieldBlock> DiscreteField::createDiscreteFieldBlock(
    const string &name,
    const shared_ptr<DiscretizationBlock> &discretizationblock) {
  assert(discretizationblock->discretization()->manifold().get() ==
         field()->manifold().get());
  auto discretefieldblock =
      DiscreteFieldBlock::create(name, shared_from_this(), discretizationblock);
  checked_emplace(m_discretefieldblocks, discretefieldblock->name(),
                  discretefieldblock, "DiscreteField", "discretefieldblocks");
  assert(discretefieldblock->invariant());
  return discretefieldblock;
}

shared_ptr<DiscreteFieldBlock>
DiscreteField::readDiscreteFieldBlock(const H5::CommonFG &loc,
                                      const string &entry) {
  auto discretefieldblock =
      DiscreteFieldBlock::create(loc, entry, shared_from_this());
  checked_emplace(m_discretefieldblocks, discretefieldblock->name(),
                  discretefieldblock, "DiscreteField", "discretefieldblocks");
  assert(discretefieldblock->invariant());
  return discretefieldblock;
}
}
