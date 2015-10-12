#include "SimulationIO.hpp"

namespace SimulationIO {

// Project

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
  tensortypes.emplace(tensortype->name, tensortype);
  assert(tensortype->invariant());
  return tensortype;
}

TensorType *Project::createTensorType(const H5::CommonFG &loc,
                                      const string &entry) {
  auto tensortype = new TensorType(loc, entry, this);
  assert(!tensortypes.count(tensortype->name));
  tensortypes.emplace(tensortype->name, tensortype);
  assert(tensortype->invariant());
  return tensortype;
}

Manifold *Project::createManifold(const string &name, int dimension) {
  assert(!manifolds.count(name));
  auto manifold = new Manifold(name, this, dimension);
  manifolds.emplace(manifold->name, manifold);
  assert(manifold->invariant());
  return manifold;
}

Manifold *Project::createManifold(const H5::CommonFG &loc,
                                  const string &entry) {
  auto manifold = new Manifold(loc, entry, this);
  assert(!manifolds.count(manifold->name));
  manifolds.emplace(manifold->name, manifold);
  assert(manifold->invariant());
  return manifold;
}

TangentSpace *Project::createTangentSpace(const string &name, int dimension) {
  assert(!tangentspaces.count(name));
  auto tangentspace = new TangentSpace(name, this, dimension);
  tangentspaces.emplace(tangentspace->name, tangentspace);
  assert(tangentspace->invariant());
  return tangentspace;
}

TangentSpace *Project::createTangentSpace(const H5::CommonFG &loc,
                                          const string &entry) {
  auto tangentspace = new TangentSpace(loc, entry, this);
  assert(!tangentspaces.count(tangentspace->name));
  tangentspaces.emplace(tangentspace->name, tangentspace);
  assert(tangentspace->invariant());
  return tangentspace;
}

Field *Project::createField(const string &name, Manifold *manifold,
                            TangentSpace *tangentspace,
                            TensorType *tensortype) {
  assert(!fields.count(name));
  auto field = new Field(name, this, manifold, tangentspace, tensortype);
  fields.emplace(field->name, field);
  assert(field->invariant());
  return field;
}

Field *Project::createField(const H5::CommonFG &loc, const string &entry) {
  auto field = new Field(loc, entry, this);
  assert(!fields.count(field->name));
  fields.emplace(field->name, field);
  assert(field->invariant());
  return field;
}

#warning "TODO"
// CoordinateSystem *Project::createCoordinateSystem(const string &name,
//                                                   Manifold *manifold) {
//   assert(!coordinatesystems.count(name));
//   auto coordinatesystem = new CoordinateSystem(name, this, manifold);
//   coordinatesystems.emplace(coordinatesystem->name, coordinatesystem);
//   assert(coordinatesystem->invariant());
//   return coordinatesystem;
// }
//
// CoordinateSystem *Project::createCoordinateSystem(const string &name,
//                                                   const H5::CommonFG &loc)
//                                                   {
//   assert(!coordinatesystems.count(name));
//   auto coordinatesystem = new CoordinateSystem(name, this, loc);
//   coordinatesystems.emplace(coordinatesystem->name, coordinatesystem);
//   assert(coordinatesystem->invariant());
//   return coordinatesystem;
// }

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
  H5::create_attribute(group, "type", "Project");
  H5::create_attribute(group, "name", name);
  // no link for parent
  H5::create_group(group, "tensortypes", tensortypes);
  H5::create_group(group, "manifolds", manifolds);
  H5::create_group(group, "tangentspaces", tangentspaces);
  H5::create_group(group, "fields", fields);
#warning "TODO"
  // H5::create_group(group, "coordiantesystems", coordinatesystems);
}

Project::Project(const H5::CommonFG &loc, const string &entry) : Common("") {
  auto group = loc.openGroup(entry);
  string type;
  H5::read_attribute(group, "type", type);
  assert(type == "Project");
  H5::read_attribute(group, "name", name);
  H5::read_group(
      group, "tensortypes", [&](const string &name, const H5::Group &group) {
        createTensorType(group, name);
      }, tensortypes);
  H5::read_group(group,
                 "manifolds", [&](const string &name, const H5::Group &group) {
                   createManifold(group, name);
                 }, manifolds);
  H5::read_group(
      group, "tangentspaces", [&](const string &name, const H5::Group &group) {
        createTangentSpace(group, name);
      }, tangentspaces);
  H5::read_group(group,
                 "fields", [&](const string &name, const H5::Group &group) {
                   createField(group, name);
                 }, fields);
#warning "TODO"
  // H5::read_group(group, "coordinatesystems",
  //                [&](const string &name, const H5::Group &group) {
  //                  createCoordinateSystem(name, group);
  //                },
  //                coordinatesystems);
}

// Tensor types

// TensorType

TensorComponent *
TensorType::createTensorComponent(const string &name,
                                  const std::vector<int> &indexvalues) {
  assert(!tensorcomponents.count(name));
  auto tensorcomponent = new TensorComponent(name, this, indexvalues);
  tensorcomponents.emplace(tensorcomponent->name, tensorcomponent);
  return tensorcomponent;
}

TensorComponent *TensorType::createTensorComponent(const H5::CommonFG &loc,
                                                   const string &entry) {
  assert(!tensorcomponents.count(name));
  auto tensorcomponent = new TensorComponent(loc, entry, this);
  tensorcomponents.emplace(tensorcomponent->name, tensorcomponent);
  return tensorcomponent;
}

ostream &TensorType::output(ostream &os, int level) const {
  os << indent(level) << "TensorType \"" << name << "\": dim=" << dimension
     << " rank=" << rank << "\n";
  for (const auto &tc : tensorcomponents)
    tc.second->output(os, level + 1);
  return os;
}

void TensorType::write(const H5::CommonFG &loc,
                       const H5::H5Location &parent) const {
  auto group = loc.createGroup(name);
  H5::create_attribute(group, "type", "TensorType");
  H5::create_attribute(group, "name", name);
  H5::createHardLink(parent, ".", group, "project");
  H5::create_attribute(group, "dimension", dimension);
  H5::create_attribute(group, "rank", rank);
#warning "TODO create link to project"
  H5::create_group(group, "tensorcomponents", tensorcomponents);
}

TensorType::TensorType(const H5::CommonFG &loc, const string &entry,
                       Project *project)
    : Common(""), project(project) {
  auto group = loc.openGroup(entry);
  string type;
  H5::read_attribute(group, "type", type);
  assert(type == "TensorType");
  H5::read_attribute(group, "name", name);
  // TODO: check link "project"
  H5::read_attribute(group, "dimension", dimension);
  H5::read_attribute(group, "rank", rank);
  H5::read_group(group, "tensorcomponents",
                 [&](const string &name, const H5::Group &group) {
                   createTensorComponent(group, name);
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

void TensorComponent::write(const H5::CommonFG &loc,
                            const H5::H5Location &parent) const {
  auto group = loc.createGroup(name);
  H5::create_attribute(group, "type", "TensorComponent");
  H5::create_attribute(group, "name", name);
  H5::createHardLink(parent, ".", group, "tensortype");
  H5::create_attribute(group, "indexvalues", indexvalues);
}

TensorComponent::TensorComponent(const H5::CommonFG &loc, const string &entry,
                                 TensorType *tensortype)
    : Common(""), tensortype(tensortype) {
  auto group = loc.openGroup(entry);
  string type;
  H5::read_attribute(group, "type", type);
  assert(type == "TensorComponent");
  H5::read_attribute(group, "name", name);
  // TODO: check link "tensortype"
  H5::read_attribute(group, "indexvalues", indexvalues);
}

// High-level continuum concepts

// Manifold

ostream &Manifold::output(ostream &os, int level) const {
  os << indent(level) << "Manifold \"" << name << "\": dim=" << dimension
     << "\n";
#warning "TODO"
  // for (const auto &d : discretizations)
  //   d.second->output(os, level + 1);
  return os;
}

void Manifold::write(const H5::CommonFG &loc,
                     const H5::H5Location &parent) const {
  auto group = loc.createGroup(name);
  H5::create_attribute(group, "type", "Manifold");
  H5::create_attribute(group, "name", name);
  H5::createHardLink(parent, ".", group, "project");
  H5::create_attribute(group, "dimension", dimension);
#warning "TODO"
  // H5::create_group(group, "discretizations", discretizations);
}

Manifold::Manifold(const H5::CommonFG &loc, const string &entry,
                   Project *project)
    : Common(""), project(project) {
  auto group = loc.openGroup(entry);
  string type;
  H5::read_attribute(group, "type", type);
  assert(type == "Manifold");
  H5::read_attribute(group, "name", name);
  // TODO: check link "project"
  H5::read_attribute(group, "dimension", dimension);
#warning "TODO"
  // H5::read_group(group, "discretizations",
  //                [&](const string &name, const H5::Group &group) {
  //                  createDiscretization(name, group);
  //                },
  //                discretizations);
}

// TangentSpace

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
  H5::create_attribute(group, "type", "TangentSpace");
  H5::create_attribute(group, "name", name);
  H5::createHardLink(parent, ".", group, "project");
  H5::create_attribute(group, "dimension", dimension);
#warning "TODO"
  // H5::create_group(group, "bases", bases);
}

TangentSpace::TangentSpace(const H5::CommonFG &loc, const string &entry,
                           Project *project)
    : Common(""), project(project) {
  auto group = loc.openGroup(entry);
  string type;
  H5::read_attribute(group, "type", type);
  assert(type == "TangentSpace");
  H5::read_attribute(group, "name", name);
  // TODO: check link "project"
  H5::read_attribute(group, "dimension", dimension);
#warning "TODO"
  // H5::read_group(group, "bases",
  //                [&](const string &name, const H5::Group &group) {
  //                  createBasis(name, group);
  //                },
  //                bases);
}

// Field

ostream &Field::output(ostream &os, int level) const {
  os << indent(level) << "Field \"" << name << "\": manifold=\""
     << manifold->name << "\" tangentspace=\"" << tangentspace->name
     << "\" tensortype=\"" << tensortype->name << "\"\n";
#warning "TODO"
  // for (const auto &df : discretefields)
  //   df.second->output(os, level + 1);
  return os;
}

void Field::write(const H5::CommonFG &loc, const H5::H5Location &parent) const {
  auto group = loc.createGroup(name);
  H5::create_attribute(group, "type", "Field");
  H5::create_attribute(group, "name", name);
  H5::createHardLink(parent, ".", group, "project");
  H5::createHardLink(parent, string("manifolds/") + manifold->name, group,
                     "manifold");
  H5::createHardLink(parent, string("tangentspaces/") + tangentspace->name,
                     group, "tangentspace");
  H5::createHardLink(parent, string("tensortypes/") + tensortype->name, group,
                     "tensortype");
#warning "TODO"
  // H5::create_group(group, "discretefields", discretefields);
}

Field::Field(const H5::CommonFG &loc, const string &entry, Project *project)
    : Common(""), project(project) {
  auto group = loc.openGroup(entry);
  string type;
  H5::read_attribute(group, "type", type);
  assert(type == "Field");
  H5::read_attribute(group, "name", name);
  // TODO: check link "project"
  // TODO: Read and interpret objects (shallowly) instead of naively only
  // looking at their names
  {
    auto obj = group.openGroup("manifold");
    string name;
    auto attr = H5::read_attribute(obj, "name", name);
    manifold = project->manifolds.at(name);
  }
  {
    auto obj = group.openGroup("tangentspace");
    string name;
    auto attr = H5::read_attribute(obj, "name", name);
    tangentspace = project->tangentspaces.at(name);
  }
  {
    auto obj = group.openGroup("tensortype");
    string name;
    auto attr = H5::read_attribute(obj, "name", name);
    tensortype = project->tensortypes.at(name);
  }
#warning "TODO"
  // H5::read_group(group, "discretefields",
  //                [&](const string &name, const H5::Group &group) {
  //                  createDiscreteField(name, group);
  //                },
  //                discretefields);
  manifold->insert(name, this);
  tangentspace->insert(name, this);
  // tensortypes->insert(this);
}

// Coordinates
}
