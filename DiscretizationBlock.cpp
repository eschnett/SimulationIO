#include "DiscretizationBlock.hpp"

#include "H5Helpers.hpp"

namespace SimulationIO {

DiscretizationBlock::DiscretizationBlock(const H5::CommonFG &loc,
                                         const string &entry,
                                         Discretization *discretization)
    : discretization(discretization) {
  auto group = loc.openGroup(entry);
  string type;
  H5::readAttribute(group, "type", type);
  assert(type == "DiscretizationBlock");
  H5::readAttribute(group, "name", name);
  // TODO: check link "discretization"
}

ostream &DiscretizationBlock::output(ostream &os, int level) const {
  os << indent(level) << "DiscretizationBlock \"" << name
     << "\": discretization=\"" << discretization->name << "\"\n";
  return os;
}

void DiscretizationBlock::write(const H5::CommonFG &loc,
                                const H5::H5Location &parent) const {
  auto group = loc.createGroup(name);
  H5::createAttribute(group, "type", "DiscretizationBlock");
  H5::createAttribute(group, "name", name);
  H5::createAttribute(group, "discretization", parent, ".");
}
}
