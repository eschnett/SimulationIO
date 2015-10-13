#include "Discretization.hpp"

#include "H5Helpers.hpp"

namespace SimulationIO {

Discretization::Discretization(const H5::CommonFG &loc, const string &entry,
                               Manifold *manifold)
    : manifold(manifold) {
  auto group = loc.openGroup(entry);
  string type;
  H5::readAttribute(group, "type", type);
  assert(type == "Discretization");
  H5::readAttribute(group, "name", name);
// TODO: check link "manifold"
#warning "TODO"
  // H5::readGroup(group, "discretizationblocks",
  //                [&](const string &name, const H5::Group &group) {
  //                  createDiscretizationBlock(name, group);
  //                },
  //                discretizationblocks);
}

ostream &Discretization::output(ostream &os, int level) const {
  os << indent(level) << "Discretization \"" << name << "\": manifold=\""
     << manifold->name << "\"\n";
#warning "TODO"
  // for (const auto &db : discretizationblocks)
  //   db.second->output(os, level + 1);
  return os;
}

void Discretization::write(const H5::CommonFG &loc,
                           const H5::H5Location &parent) const {
  auto group = loc.createGroup(name);
  H5::createAttribute(group, "type", "Discretization");
  H5::createAttribute(group, "name", name);
  H5::createAttribute(group, "manifold", parent, ".");
#warning "TODO"
  // H5::createGroup(group, "discretizationblocks", discretizationblocks);
}
}
