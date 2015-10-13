#include "TangentSpace.hpp"

#include "H5Helpers.hpp"

namespace SimulationIO {

TangentSpace::TangentSpace(const H5::CommonFG &loc, const string &entry,
                           Project *project)
    : project(project) {
  auto group = loc.openGroup(entry);
  string type;
  H5::readAttribute(group, "type", type);
  assert(type == "TangentSpace");
  H5::readAttribute(group, "name", name);
  // TODO: check link "project"
  H5::readAttribute(group, "dimension", dimension);
#warning "TODO"
  // H5::readGroup(group, "bases",
  //                [&](const string &name, const H5::Group &group) {
  //                  createBasis(name, group);
  //                },
  //                bases);
}

ostream &TangentSpace::output(ostream &os, int level) const {
  os << indent(level) << "TangentSpace \"" << name << "\": dim=" << dimension
     << "\n";
#warning "TODO"
  // for (const auto &b : bases)
  //   b.second->output(os, level + 1);
  return os;
}

void TangentSpace::write(const H5::CommonFG &loc,
                         const H5::H5Location &parent) const {
  auto group = loc.createGroup(name);
  H5::createAttribute(group, "type", "TangentSpace");
  H5::createAttribute(group, "name", name);
  H5::createAttribute(group, "project", parent, ".");
  H5::createAttribute(group, "dimension", dimension);
#warning "TODO"
  // H5::createGroup(group, "bases", bases);
}
}
