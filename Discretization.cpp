#include "Discretization.hpp"

#include "DiscretizationBlock.hpp"

#include "H5Helpers.hpp"

namespace SimulationIO {

Discretization::Discretization(const H5::CommonFG &loc, const string &entry,
                               Manifold *manifold)
    : manifold(manifold) {
  auto group = loc.openGroup(entry);
  assert(H5::readAttribute<string>(
             group, "type", manifold->project->enumtype) == "Discretization");
  H5::readAttribute(group, "name", name);
  assert(H5::readGroupAttribute<string>(group, "manifold", "name") ==
         manifold->name);
  H5::readGroup(group, "discretizationblocks",
                [&](const H5::Group &group, const string &name) {
                  createDiscretizationBlock(group, name);
                });
}

ostream &Discretization::output(ostream &os, int level) const {
  os << indent(level) << "Discretization \"" << name << "\": manifold=\""
     << manifold->name << "\"\n";
  for (const auto &db : discretizationblocks)
    db.second->output(os, level + 1);
  return os;
}

void Discretization::write(const H5::CommonFG &loc,
                           const H5::H5Location &parent) const {
  auto group = loc.createGroup(name);
  H5::createAttribute(group, "type", manifold->project->enumtype,
                      "Discretization");
  H5::createAttribute(group, "name", name);
  H5::createHardLink(group, "manifold", parent, ".");
  H5::createGroup(group, "discretizationblocks", discretizationblocks);
}

DiscretizationBlock *
Discretization::createDiscretizationBlock(const string &name) {
  auto discretizationblock = new DiscretizationBlock(name, this);
  checked_emplace(discretizationblocks, discretizationblock->name,
                  discretizationblock);
  assert(discretizationblock->invariant());
  return discretizationblock;
}

DiscretizationBlock *
Discretization::createDiscretizationBlock(const H5::CommonFG &loc,
                                          const string &entry) {
  auto discretizationblock = new DiscretizationBlock(loc, entry, this);
  checked_emplace(discretizationblocks, discretizationblock->name,
                  discretizationblock);
  assert(discretizationblock->invariant());
  return discretizationblock;
}
}
