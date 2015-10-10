#include "SimulationIO.hpp"

namespace SimulationIO {

// Project

Project *createProject(const string &name) {
  auto project = new Project(name);
  assert(project->invariant());
  return project;
}
Project *createProject(const string &name, const H5::CommonFG &loc) {
  auto project = new Project(name, loc);
  assert(project->invariant());
  return project;
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

TensorType *Project::createTensorType(const string &name, int dimension,
                                      int rank) {
  assert(!tensortypes.count(name));
  auto tensortype = new TensorType(name, this, dimension, rank);
  // TODO: use emplace
  tensortypes[name] = tensortype;
  assert(tensortype->invariant());
  return tensortype;
}

TensorType *Project::createTensorType(const string &name,
                                      const H5::CommonFG &loc) {
  assert(!tensortypes.count(name));
  auto tensortype = new TensorType(name, this, loc);
  // TODO: use emplace
  tensortypes[name] = tensortype;
  assert(tensortype->invariant());
  return tensortype;
}

Manifold *Project::createManifold(const string &name, int dimension) {
  assert(!manifolds.count(name));
  auto manifold = new Manifold(name, this, dimension);
  manifolds[name] = manifold;
  assert(manifold->invariant());
  return manifold;
}

Manifold *Project::createManifold(const string &name, const H5::CommonFG &loc) {
  assert(!manifolds.count(name));
  auto manifold = new Manifold(name, this, loc);
  manifolds[name] = manifold;
  assert(manifold->invariant());
  return manifold;
}

TangentSpace *Project::createTangentSpace(const string &name, int dimension) {
  assert(!tangentspaces.count(name));
  auto tangentspace = new TangentSpace(name, this, dimension);
  tangentspaces[name] = tangentspace;
  assert(tangentspace->invariant());
  return tangentspace;
}

TangentSpace *Project::createTangentSpace(const string &name,
                                          const H5::CommonFG &loc) {
  assert(!tangentspaces.count(name));
  auto tangentspace = new TangentSpace(name, this, loc);
  tangentspaces[name] = tangentspace;
  assert(tangentspace->invariant());
  return tangentspace;
}

Field *Project::createField(const string &name, Manifold *manifold,
                            TangentSpace *tangentspace,
                            TensorType *tensortype) {
  assert(!fields.count(name));
  auto field = new Field(name, this, manifold, tangentspace, tensortype);
  fields[name] = field;
  assert(field->invariant());
  return field;
}

Field *Project::createField(const string &name, const H5::CommonFG &loc) {
  assert(!fields.count(name));
  auto field = new Field(name, this, loc);
  fields[name] = field;
  assert(field->invariant());
  return field;
}

#warning "TODO"
// CoordinateSystem *Project::createCoordinateSystem(const string &name,
//                                                   Manifold *manifold) {
//   assert(!coordinatesystems.count(name));
//   auto coordinatesystem = new CoordinateSystem(name, this, manifold);
//   coordinatesystems[name] = coordinatesystem;
//   assert(coordinatesystem->invariant());
//   return coordinatesystem;
// }
//
// CoordinateSystem *Project::createCoordinateSystem(const string &name,
//                                                   const H5::CommonFG &loc) {
//   assert(!coordinatesystems.count(name));
//   auto coordinatesystem = new CoordinateSystem(name, this, loc);
//   coordinatesystems[name] = coordinatesystem;
//   assert(coordinatesystem->invariant());
//   return coordinatesystem;
// }

ostream &Project::output(ostream &os, int level) const {
  os << indent(level) << "Project \"" << name << "\"\n";
  os << indent(level) << "tensortypes:\n";
  for (const auto &tt : tensortypes)
    tt.second->output(os, level + 1);
#warning "TODO"
  // os << indent(level) << "manifolds:\n";
  // for (const auto &m : manifolds)
  //   m.second->output(os, level + 1);
  // os << indent(level) << "tangenspaces:\n";
  // for (const auto &ts : tangentspaces)
  //   ts.second->output(os, level + 1);
  // os << indent(level) << "fields:\n";
  // for (const auto &f : fields)
  //   f.second->output(os, level + 1);
  // os << indent(level) << "coordinatesystems:\n";
  // for (const auto &cs : coordinatesystems)
  //   cs.second->output(os, level + 1);
  return os;
}

void Project::write(H5::CommonFG &loc) const {
  // Note: We don't create a group for the project. We assume that "loc" may be
  // a file, and we then generate top-level entries for this file.
  H5::create_group(loc, "tensortypes", tensortypes);
  H5::create_group(loc, "manifolds", manifolds);
  H5::create_group(loc, "tangentspaces", tangentspaces);
  H5::create_group(loc, "fields", fields);
#warning "TODO"
  // H5::create_group(loc, "coordiantesystems", coordinatesystems);
}

Project::Project(const string &name, const H5::CommonFG &loc) : Common(name) {
  assert(invariant());
  H5::read_group(
      loc, "tensortypes", [&](const string &name, const H5::Group &group) {
        createTensorType(name, group);
      }, tensortypes);
#warning "TODO"
  // H5::read_group(loc, "manifolds", this, manifolds);
  // H5::read_group(loc, "tangentspaces", this, tangentspaces);
  // H5::read_group(loc, "fields", this, fields);
  // H5::read_group(loc, "coordinatesystems", this, coordinatesystems);
}

// Tensor types

// TensorType

TensorComponent *
TensorType::createTensorComponent(const string &name,
                                  const std::vector<int> &indexvalues) {
  assert(!tensorcomponents.count(name));
  auto tensorcomponent = new TensorComponent(name, this, indexvalues);
  tensorcomponents[name] = tensorcomponent;
  return tensorcomponent;
}

TensorComponent *TensorType::createTensorComponent(const string &name,
                                                   const H5::CommonFG &loc) {
  assert(!tensorcomponents.count(name));
  auto tensorcomponent = new TensorComponent(name, this, loc);
  tensorcomponents[name] = tensorcomponent;
  return tensorcomponent;
}

ostream &TensorType::output(ostream &os, int level) const {
  os << indent(level) << "TensorType \"" << name << "\": dim=" << dimension
     << " rank=" << rank << "\n";
  for (const auto &tc : tensorcomponents)
    tc.second->output(os, level + 1);
  return os;
}

void TensorType::write(H5::CommonFG &loc) const {
  auto group = loc.createGroup(name);
  H5::create_attribute(group, "dimension", dimension);
  H5::create_attribute(group, "rank", rank);
  H5::create_group(group, "tensorcomponents", tensorcomponents);
}

TensorType::TensorType(const string &name, Project *project,
                       const H5::CommonFG &loc)
    : Common(name), project(project) {
  auto group = loc.openGroup(name);
  H5::read_attribute(group, "dimension", dimension);
  H5::read_attribute(group, "rank", rank);
  H5::read_group(group, "tensorcomponents",
                 [&](const string &name, const H5::Group &group) {
                   createTensorComponent(name, group);
                 },
                 tensorcomponents);
}

// TensorComponent

ostream &TensorComponent::output(ostream &os, int level) const {
  os << indent(level) << "TensorComponent \"" << name << "\": tensortype=\""
     << tensortype->name << "\" indexvalues=[";
  for (int i = 0; i < int(indexvalues.size()); ++i) {
    if (i > 0)
      os << ",";
    os << indexvalues[i];
  }
  os << "]\n";
  return os;
}

void TensorComponent::write(H5::CommonFG &loc) const {
  auto group = loc.createGroup(name);
  H5::create_attribute(group, "indexvalues", indexvalues);
}

TensorComponent::TensorComponent(const string &name, TensorType *tensortype,
                                 const H5::CommonFG &loc)
    : Common(name), tensortype(tensortype) {
  auto group = loc.openGroup(name);
  H5::read_attribute(group, "indexvalues", indexvalues);
}

// High-level continuum concepts

// Manifold

ostream &Manifold::output(ostream &os, int level) const { assert(0); }

void Manifold::write(H5::CommonFG &loc) const { assert(0); }

Manifold::Manifold(const string &name, Project *project,
                   const H5::CommonFG &loc)
    : Common(name), project(project) {
  auto group = loc.openGroup(name);
  H5::read_attribute(group, "dimension", dimension);
#warning "TODO"
  // H5::read_group(group, "discretizations",
  //                [&](const string &name, const H5::Group &group) {
  //                  createDiscretization(name, group);
  //                },
  //                discretizations);
}

// TangentSpace

ostream &TangentSpace::output(ostream &os, int level) const { assert(0); }

void TangentSpace::write(H5::CommonFG &loc) const { assert(0); }

TangentSpace::TangentSpace(const string &name, Project *project,
                           const H5::CommonFG &loc)
    : Common(name), project(project) {
  auto group = loc.openGroup(name);
  H5::read_attribute(group, "dimension", dimension);
#warning "TODO"
  // H5::read_group(group, "bases",
  //                [&](const string &name, const H5::Group &group) {
  //                  createBasis(name, group);
  //                },
  //                bases);
}

// Field

ostream &Field::output(ostream &os, int level) const { assert(0); }

void Field::write(H5::CommonFG &loc) const { assert(0); }

Field::Field(const string &name, Project *project, const H5::CommonFG &loc)
    : Common(name), project(project) {
  auto group = loc.openGroup(name);
#warning "TODO: manifold, tangentspace, tensortype"
#warning "TODO"
  // H5::read_group(group, "discretefields",
  //                [&](const string &name, const H5::Group &group) {
  //                  createDiscreteField(name, group);
  //                },
  //                discretefields);
}

// Coordinates
}
