#include "CoordinateSystem.hpp"

#include "CoordinateField.hpp"

#include "H5Helpers.hpp"

namespace SimulationIO {

void CoordinateSystem::read(const H5::CommonFG &loc, const string &entry,
                            const shared_ptr<Project> &project) {
  this->project = project;
  auto group = loc.openGroup(entry);
  assert(H5::readAttribute<string>(group, "type", project->enumtype) ==
         "CoordinateSystem");
  H5::readAttribute(group, "name", name);
  assert(H5::readGroupAttribute<string>(group, "project", "name") ==
         project->name);
  configuration = project->configurations.at(
      H5::readGroupAttribute<string>(group, "configuration", "name"));
  manifold = project->manifolds.at(
      H5::readGroupAttribute<string>(group, "manifold", "name"));
  assert(H5::readGroupAttribute<string>(
             group, string("project/coordinatesystems/") + name, "name") ==
         name);
  H5::readGroup(group, "coordinatefields",
                [&](const H5::Group &group, const string &name) {
                  readCoordinateField(group, name);
                });
#warning "TODO: check group directions"
  configuration->insert(name, shared_from_this());
  manifold->insert(name, shared_from_this());
}

ostream &CoordinateSystem::output(ostream &os, int level) const {
  os << indent(level) << "CoordinateSystem " << quote(name)
     << ": Configuration " << quote(configuration->name) << " Project "
     << quote(project.lock()->name) << " Manifold " << quote(manifold->name)
     << "\n";
  for (const auto &cf : directions)
    cf.second->output(os, level + 1);
  return os;
}

void CoordinateSystem::write(const H5::CommonFG &loc,
                             const H5::H5Location &parent) const {
  assert(invariant());
  auto group = loc.createGroup(name);
  H5::createAttribute(group, "type", project.lock()->enumtype,
                      "CoordinateSystem");
  H5::createAttribute(group, "name", name);
  // H5::createHardLink(group, "project", parent, ".");
  H5::createHardLink(group, "..", parent, ".");
  H5::createSoftLink(group, "project", "..");
  // H5::createHardLink(group, "configuration", parent,
  //                    string("configurations/") + configuration->name);
  H5::createSoftLink(group, "configuration",
                     string("../configurations/") + configuration->name);
  H5::createHardLink(group, string("project/configurations/") +
                                configuration->name + "/coordinatesystems",
                     name, group, ".");
  // H5::createHardLink(group, "manifold", parent,
  //                    string("manifolds/") + manifold->name);
  H5::createSoftLink(group, "manifold",
                     string("../manifolds/") + manifold->name);
  H5::createHardLink(group, string("project/manifolds/") + manifold->name +
                                "/coordinatesystems",
                     name, group, ".");
  H5::createGroup(group, "coordinatefields", coordinatefields);
#warning "TODO: output directions"
}

shared_ptr<CoordinateField>
CoordinateSystem::createCoordinateField(const string &name, int direction,
                                        const shared_ptr<Field> &field) {
  auto coordinatefield =
      CoordinateField::create(name, shared_from_this(), direction, field);
  checked_emplace(coordinatefields, coordinatefield->name, coordinatefield);
  checked_emplace(directions, coordinatefield->direction, coordinatefield);
  assert(coordinatefield->invariant());
  return coordinatefield;
}

shared_ptr<CoordinateField>
CoordinateSystem::readCoordinateField(const H5::CommonFG &loc,
                                      const string &entry) {
  auto coordinatefield =
      CoordinateField::create(loc, entry, shared_from_this());
  checked_emplace(coordinatefields, coordinatefield->name, coordinatefield);
  checked_emplace(directions, coordinatefield->direction, coordinatefield);
  assert(coordinatefield->invariant());
  return coordinatefield;
}
}
