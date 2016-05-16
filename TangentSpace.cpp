#include "TangentSpace.hpp"

#include "Basis.hpp"
#include "Field.hpp"

#include "H5Helpers.hpp"

namespace SimulationIO {

void TangentSpace::read(const H5::CommonFG &loc, const string &entry,
                        const shared_ptr<Project> &project) {
  this->project = project;
  auto group = loc.openGroup(entry);
  assert(H5::readAttribute<string>(group, "type", project->enumtype) ==
         "TangentSpace");
  H5::readAttribute(group, "name", name);
  assert(H5::readGroupAttribute<string>(group, "project", "name") ==
         project->name);
  configuration = project->configurations.at(
      H5::readGroupAttribute<string>(group, "configuration", "name"));
  assert(H5::readGroupAttribute<string>(
             group, string("configuration/tangentspaces/") + name, "name") ==
         name);
  H5::readAttribute(group, "dimension", dimension);
  H5::readGroup(group, "bases",
                [&](const H5::Group &group, const string &name) {
                  readBasis(group, name);
                });
  // Cannot check "fields" since fields have not been read yet
  // assert(H5::checkGroupNames(group, "fields", fields));
  configuration->insert(name, shared_from_this());
}

ostream &TangentSpace::output(ostream &os, int level) const {
  os << indent(level) << "TangentSpace " << quote(name) << ": Configuration "
     << quote(configuration->name) << " dim=" << dimension << "\n";
  for (const auto &b : bases)
    b.second->output(os, level + 1);
  for (const auto &f : fields)
    os << indent(level + 1) << "Field " << quote(f.second.lock()->name) << "\n";
  return os;
}

void TangentSpace::write(const H5::CommonFG &loc,
                         const H5::H5Location &parent) const {
  assert(invariant());
  auto group = loc.createGroup(name);
  H5::createAttribute(group, "type", project.lock()->enumtype, "TangentSpace");
  H5::createAttribute(group, "name", name);
  // H5::createHardLink(group, "project", parent, ".");
  H5::createHardLink(group, "..", parent, ".");
  H5::createSoftLink(group, "project", "..");
  // H5::createHardLink(group, "configuration", parent,
  //                    string("configurations/") + configuration->name);
  H5::createSoftLink(group, "configuration",
                     string("../configurations/") + configuration->name);
  H5::createHardLink(group, string("project/configurations/") +
                                configuration->name + "/tangentspaces",
                     name, group, ".");
  H5::createAttribute(group, "dimension", dimension);
  H5::createGroup(group, "bases", bases);
  group.createGroup("fields");
}

shared_ptr<Basis>
TangentSpace::createBasis(const string &name,
                          const shared_ptr<Configuration> &configuration) {
  auto basis = Basis::create(name, shared_from_this(), configuration);
  checked_emplace(bases, basis->name, basis);
  assert(basis->invariant());
  return basis;
}

shared_ptr<Basis> TangentSpace::readBasis(const H5::CommonFG &loc,
                                          const string &entry) {
  auto basis = Basis::create(loc, entry, shared_from_this());
  checked_emplace(bases, basis->name, basis);
  assert(basis->invariant());
  return basis;
}
}
