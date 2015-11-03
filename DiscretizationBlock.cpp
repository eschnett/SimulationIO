#include "DiscretizationBlock.hpp"

#include "H5Helpers.hpp"

namespace SimulationIO {

void DiscretizationBlock::read(
    const H5::CommonFG &loc, const string &entry,
    const shared_ptr<Discretization> &discretization) {
  this->discretization = discretization;
  auto group = loc.openGroup(entry);
  assert(H5::readAttribute<string>(
             group, "type",
             discretization->manifold.lock()->project.lock()->enumtype) ==
         "DiscretizationBlock");
  H5::readAttribute(group, "name", name);
  assert(H5::readGroupAttribute<string>(group, "discretization", "name") ==
         discretization->name);
  if (group.attrExists("offset"))
    H5::readAttribute(group, "offset", offset);
}

ostream &DiscretizationBlock::output(ostream &os, int level) const {
  os << indent(level) << "DiscretizationBlock " << quote(name)
     << ": Discretization " << quote(discretization.lock()->name);
  if (!offset.empty()) {
    os << " offset=[";
    for (int d = 0; d < int(offset.size()); ++d) {
      if (d > 0)
        os << ",";
      os << offset.at(d);
    }
    os << "]";
  }
  os << "\n";
  return os;
}

void DiscretizationBlock::write(const H5::CommonFG &loc,
                                const H5::H5Location &parent) const {
  assert(invariant());
  auto group = loc.createGroup(name);
  H5::createAttribute(
      group, "type",
      discretization.lock()->manifold.lock()->project.lock()->enumtype,
      "DiscretizationBlock");
  H5::createAttribute(group, "name", name);
  H5::createHardLink(group, "discretization", parent, ".");
  if (!offset.empty())
    H5::createAttribute(group, "offset", offset);
}
}
