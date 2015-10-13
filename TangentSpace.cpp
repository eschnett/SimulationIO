#include "TangentSpace.hpp"

#include "Basis.hpp"

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
  H5::readGroup(group,
                "bases", [&](const string &name, const H5::Group &group) {
                  createBasis(group, name);
                }, bases);
}

ostream &TangentSpace::output(ostream &os, int level) const {
  os << indent(level) << "TangentSpace \"" << name << "\": dim=" << dimension
     << "\n";
  for (const auto &b : bases)
    b.second->output(os, level + 1);
  return os;
}

void TangentSpace::write(const H5::CommonFG &loc,
                         const H5::H5Location &parent) const {
  auto group = loc.createGroup(name);
  H5::createAttribute(group, "type", "TangentSpace");
  H5::createAttribute(group, "name", name);
  H5::createHardLink(group, "project", parent, ".");
  // H5::createAttribute(group, "project", parent, ".");
  H5::createAttribute(group, "dimension", dimension);
  H5::createGroup(group, "bases", bases);
}

Basis *TangentSpace::createBasis(const string &name) {
  auto basis = new Basis(name, this);
  checked_emplace(bases, basis->name, basis);
  return basis;
}

Basis *TangentSpace::createBasis(const H5::CommonFG &loc, const string &entry) {
  auto basis = new Basis(loc, entry, this);
  checked_emplace(bases, basis->name, basis);
  return basis;
}
}
