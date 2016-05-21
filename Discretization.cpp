#include "Discretization.hpp"

#include "DiscretizationBlock.hpp"

#include "H5Helpers.hpp"

namespace SimulationIO {

void Discretization::read(const H5::CommonFG &loc, const string &entry,
                          const shared_ptr<Manifold> &manifold) {
  this->manifold = manifold;
  auto group = loc.openGroup(entry);
  assert(H5::readAttribute<string>(
             group, "type", manifold->project()->enumtype) == "Discretization");
  H5::readAttribute(group, "name", m_name);
  assert(H5::readGroupAttribute<string>(group, "manifold", "name") ==
         manifold->name());
  configuration = manifold->project()->configurations.at(
      H5::readGroupAttribute<string>(group, "configuration", "name"));
  assert(H5::readGroupAttribute<string>(
             group, string("configuration/discretizations/") + name(),
             "name") == name());
  H5::readGroup(group, "discretizationblocks",
                [&](const H5::Group &group, const string &name) {
                  readDiscretizationBlock(group, name);
                });
  configuration->insert(name(), shared_from_this());
}

ostream &Discretization::output(ostream &os, int level) const {
  os << indent(level) << "Discretization " << quote(name())
     << ": Configuration " << quote(configuration->name()) << " Manifold "
     << quote(manifold.lock()->name()) << "\n";
  for (const auto &db : discretizationblocks)
    db.second->output(os, level + 1);
  return os;
}

void Discretization::write(const H5::CommonFG &loc,
                           const H5::H5Location &parent) const {
  assert(invariant());
  auto group = loc.createGroup(name());
  H5::createAttribute(group, "type", manifold.lock()->project()->enumtype,
                      "Discretization");
  H5::createAttribute(group, "name", name());
  // H5::createHardLink(group, "manifold", parent, ".");
  H5::createHardLink(group, "..", parent, ".");
  H5::createSoftLink(group, "manifold", "..");
  // H5::createHardLink(group, "configuration", parent,
  //                    string("project/configurations/") +
  //                    configuration->name());
  H5::createSoftLink(group, "configuration",
                     string("../project/configurations/") +
                         configuration->name());
  H5::createHardLink(group, string("manifold/project/configurations/") +
                                configuration->name() + "/discretizations",
                     name(), group, ".");
  H5::createGroup(group, "discretizationblocks", discretizationblocks);
  group.createGroup("child_discretizations");
  group.createGroup("parent_discretizations");
}

shared_ptr<DiscretizationBlock>
Discretization::createDiscretizationBlock(const string &name) {
  auto discretizationblock =
      DiscretizationBlock::create(name, shared_from_this());
  checked_emplace(discretizationblocks, discretizationblock->name(),
                  discretizationblock);
  assert(discretizationblock->invariant());
  return discretizationblock;
}

shared_ptr<DiscretizationBlock>
Discretization::readDiscretizationBlock(const H5::CommonFG &loc,
                                        const string &entry) {
  auto discretizationblock =
      DiscretizationBlock::create(loc, entry, shared_from_this());
  checked_emplace(discretizationblocks, discretizationblock->name(),
                  discretizationblock);
  assert(discretizationblock->invariant());
  return discretizationblock;
}
}
