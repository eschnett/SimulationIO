#include "DiscretizationBlock.hpp"

#include "H5Helpers.hpp"

namespace SimulationIO {

DiscretizationBlock::DiscretizationBlock(const H5::CommonFG &loc,
                                         const string &entry,
                                         Discretization *discretization)
    : discretization(discretization) {
  auto group = loc.openGroup(entry);
  assert(H5::readAttribute<string>(
             group, "type", discretization->manifold->project->enumtype) ==
         "DiscretizationBlock");
  H5::readAttribute(group, "name", name);
  assert(H5::readGroupAttribute<string>(group, "discretization", "name") ==
         discretization->name);
}

ostream &DiscretizationBlock::output(ostream &os, int level) const {
  os << indent(level) << "DiscretizationBlock \"" << name
     << "\": Discretization \"" << discretization->name << "\"\n";
  return os;
}

void DiscretizationBlock::write(const H5::CommonFG &loc,
                                const H5::H5Location &parent) const {
  auto group = loc.createGroup(name);
  H5::createAttribute(group, "type",
                      discretization->manifold->project->enumtype,
                      "DiscretizationBlock");
  H5::createAttribute(group, "name", name);
  H5::createHardLink(group, "discretization", parent, ".");
}
}
