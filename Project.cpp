#include "Project.hpp"

#include "Field.hpp"
#include "Manifold.hpp"
#include "TangentSpace.hpp"
#include "TensorType.hpp"

#include "H5Helpers.hpp"

namespace SimulationIO {

Project *createProject(const string &name) {
  auto project = new Project(name);
  assert(project->invariant());
  return project;
}
Project *createProject(const H5::CommonFG &loc, const string &entry) {
  auto project = new Project(loc, entry);
  assert(project->invariant());
  return project;
}

Project::Project(const H5::CommonFG &loc, const string &entry) {
  auto group = loc.openGroup(entry);
  string type;
  H5::readAttribute(group, "type", type);
  assert(type == "Project");
  H5::readAttribute(group, "name", name);
  H5::readGroup(group,
                "tensortypes", [&](const string &name, const H5::Group &group) {
                  createTensorType(group, name);
                }, tensortypes);
  H5::readGroup(group,
                "manifolds", [&](const string &name, const H5::Group &group) {
                  createManifold(group, name);
                }, manifolds);
  H5::readGroup(
      group, "tangentspaces", [&](const string &name, const H5::Group &group) {
        createTangentSpace(group, name);
      }, tangentspaces);
  H5::readGroup(group,
                "fields", [&](const string &name, const H5::Group &group) {
                  createField(group, name);
                }, fields);
#warning "TODO"
  // H5::readGroup(group, "coordinatesystems",
  //                [&](const string &name, const H5::Group &group) {
  //                  createCoordinateSystem(name, group);
  //                },
  //                coordinatesystems);
}

void Project::createStandardTensortypes() {
  auto s3d = createTensorType("Scalar3D", 3, 0);
  s3d->createTensorComponent("scalar", vector<int>{});

  auto v3d = createTensorType("Vector3D", 3, 1);
  v3d->createTensorComponent("0", {0});
  v3d->createTensorComponent("1", {1});
  v3d->createTensorComponent("2", {2});

  auto st3d = createTensorType("SymmetricTensor3D", 3, 2);
  st3d->createTensorComponent("00", {0, 0});
  st3d->createTensorComponent("01", {0, 1});
  st3d->createTensorComponent("02", {0, 2});
  st3d->createTensorComponent("11", {1, 1});
  st3d->createTensorComponent("12", {1, 2});
  st3d->createTensorComponent("22", {2, 2});
}

ostream &Project::output(ostream &os, int level) const {
  os << indent(level) << "Project \"" << name << "\"\n";
  os << indent(level) << "tensortypes:\n";
  for (const auto &tt : tensortypes)
    tt.second->output(os, level + 1);
  os << indent(level) << "manifolds:\n";
  for (const auto &m : manifolds)
    m.second->output(os, level + 1);
  os << indent(level) << "tangenspaces:\n";
  for (const auto &ts : tangentspaces)
    ts.second->output(os, level + 1);
  os << indent(level) << "fields:\n";
  for (const auto &f : fields)
    f.second->output(os, level + 1);
#warning "TODO"
  // os << indent(level) << "coordinatesystems:\n";
  // for (const auto &cs : coordinatesystems)
  //   cs.second->output(os, level + 1);
  return os;
}

void Project::write(const H5::CommonFG &loc,
                    const H5::H5Location &parent) const {
  auto group = loc.createGroup(name);
  H5::createAttribute(group, "type", "Project");
  H5::createAttribute(group, "name", name);
  // no link for parent
  H5::createGroup(group, "tensortypes", tensortypes);
  H5::createGroup(group, "manifolds", manifolds);
  H5::createGroup(group, "tangentspaces", tangentspaces);
  H5::createGroup(group, "fields", fields);
#warning "TODO"
  // H5::createGroup(group, "coordiantesystems", coordinatesystems);
}

TensorType *Project::createTensorType(const string &name, int dimension,
                                      int rank) {
  auto tensortype = new TensorType(name, this, dimension, rank);
  checked_emplace(tensortypes, tensortype->name, tensortype);
  assert(tensortype->invariant());
  return tensortype;
}

TensorType *Project::createTensorType(const H5::CommonFG &loc,
                                      const string &entry) {
  auto tensortype = new TensorType(loc, entry, this);
  checked_emplace(tensortypes, tensortype->name, tensortype);
  assert(tensortype->invariant());
  return tensortype;
}

Manifold *Project::createManifold(const string &name, int dimension) {
  auto manifold = new Manifold(name, this, dimension);
  checked_emplace(manifolds, manifold->name, manifold);
  assert(manifold->invariant());
  return manifold;
}

Manifold *Project::createManifold(const H5::CommonFG &loc,
                                  const string &entry) {
  auto manifold = new Manifold(loc, entry, this);
  checked_emplace(manifolds, manifold->name, manifold);
  assert(manifold->invariant());
  return manifold;
}

TangentSpace *Project::createTangentSpace(const string &name, int dimension) {
  auto tangentspace = new TangentSpace(name, this, dimension);
  checked_emplace(tangentspaces, tangentspace->name, tangentspace);
  assert(tangentspace->invariant());
  return tangentspace;
}

TangentSpace *Project::createTangentSpace(const H5::CommonFG &loc,
                                          const string &entry) {
  auto tangentspace = new TangentSpace(loc, entry, this);
  checked_emplace(tangentspaces, tangentspace->name, tangentspace);
  assert(tangentspace->invariant());
  return tangentspace;
}

Field *Project::createField(const string &name, Manifold *manifold,
                            TangentSpace *tangentspace,
                            TensorType *tensortype) {
  auto field = new Field(name, this, manifold, tangentspace, tensortype);
  checked_emplace(fields, field->name, field);
  assert(field->invariant());
  return field;
}

Field *Project::createField(const H5::CommonFG &loc, const string &entry) {
  auto field = new Field(loc, entry, this);
  checked_emplace(fields, field->name, field);
  assert(field->invariant());
  return field;
}

#warning "TODO"
// CoordinateSystem *Project::createCoordinateSystem(const string &name,
//                                                   Manifold *manifold) {
//   auto coordinatesystem = new CoordinateSystem(name, this, manifold);
//   checked_emplace(coordinatesystems,coordinatesystem->name,
//   coordinatesystem);
//   assert(coordinatesystem->invariant());
//   return coordinatesystem;
// }
//
// CoordinateSystem *Project::createCoordinateSystem(const string &name,
//                                                   const H5::CommonFG &loc)
//                                                   {
//   auto coordinatesystem = new CoordinateSystem(name, this, loc);
//   checked_emplace(coordinatesystems, coordinatesystem->name,
//   coordinatesystem);
//   assert(coordinatesystem->invariant());
//   return coordinatesystem;
// }
}