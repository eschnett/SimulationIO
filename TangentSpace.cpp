#include "TangentSpace.hpp"

#include "Basis.hpp"
#include "Field.hpp"

#include "H5Helpers.hpp"

namespace SimulationIO {

TangentSpace::TangentSpace(const H5::CommonFG &loc, const string &entry,
                           Project *project)
    : project(project) {
  auto group = loc.openGroup(entry);
  assert(H5::readAttribute<string>(group, "type", project->enumtype) ==
         "TangentSpace");
  H5::readAttribute(group, "name", name);
  assert(H5::readGroupAttribute<string>(group, "project", "name") ==
         project->name);
  H5::readAttribute(group, "dimension", dimension);
  H5::readGroup(group, "bases",
                [&](const H5::Group &group, const string &name) {
                  createBasis(group, name);
                });
  // Cannot check "fields" since fields have not been read yet
  // assert(H5::checkGroupNames(group, "fields", fields));
}

ostream &TangentSpace::output(ostream &os, int level) const {
  os << indent(level) << "TangentSpace \"" << name << "\": dim=" << dimension
     << "\n";
  for (const auto &b : bases)
    b.second->output(os, level + 1);
  for (const auto &f : fields)
    os << indent(level + 1) << "Field \"" << f.second->name << "\"\n";
  return os;
}

void TangentSpace::write(const H5::CommonFG &loc,
                         const H5::H5Location &parent) const {
  auto group = loc.createGroup(name);
  H5::createAttribute(group, "type", project->enumtype, "TangentSpace");
  H5::createAttribute(group, "name", name);
  H5::createHardLink(group, "project", parent, ".");
  H5::createAttribute(group, "dimension", dimension);
  H5::createGroup(group, "bases", bases);
  group.createGroup("fields");
}

Basis *TangentSpace::createBasis(const string &name) {
  auto basis = new Basis(name, this);
  checked_emplace(bases, basis->name, basis);
  assert(basis->invariant());
  return basis;
}

Basis *TangentSpace::createBasis(const H5::CommonFG &loc, const string &entry) {
  auto basis = new Basis(loc, entry, this);
  checked_emplace(bases, basis->name, basis);
  assert(basis->invariant());
  return basis;
}
}
