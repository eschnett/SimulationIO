#include "DiscretizationBlock.hpp"

#include "H5Helpers.hpp"

#include <algorithm>

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
  std::reverse(offset.begin(), offset.end());
}

ostream &DiscretizationBlock::output(ostream &os, int level) const {
  os << indent(level) << "DiscretizationBlock " << quote(name)
     << ": Discretization " << quote(discretization.lock()->name);
  if (!offset.empty())
    os << " offset=" << offset << "\n";
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
  if (!offset.empty()) {
    auto tmp_offset = offset;
    std::reverse(tmp_offset.begin(), tmp_offset.end());
    H5::createAttribute(group, "offset", tmp_offset);
  }
}
}
