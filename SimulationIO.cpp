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

Project::Project(const H5::CommonFG &loc, const string &entry) : Common("") {
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

// Tensor types

// TensorType

TensorComponent *
TensorType::createTensorComponent(const string &name,
                                  const std::vector<int> &indexvalues) {
  auto tensorcomponent = new TensorComponent(name, this, indexvalues);
  checked_emplace(tensorcomponents, tensorcomponent->name, tensorcomponent);
  return tensorcomponent;
}

TensorComponent *TensorType::createTensorComponent(const H5::CommonFG &loc,
                                                   const string &entry) {
  auto tensorcomponent = new TensorComponent(loc, entry, this);
  checked_emplace(tensorcomponents, tensorcomponent->name, tensorcomponent);
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
  H5::createAttribute(group, "type", "TensorType");
  H5::createAttribute(group, "name", name);
  H5::createHardLink(parent, ".", group, "project");
  H5::createAttribute(group, "dimension", dimension);
  H5::createAttribute(group, "rank", rank);
#warning "TODO create link to project"
  H5::createGroup(group, "tensorcomponents", tensorcomponents);
}

TensorType::TensorType(const H5::CommonFG &loc, const string &entry,
                       Project *project)
    : Common(""), project(project) {
  auto group = loc.openGroup(entry);
  string type;
  H5::readAttribute(group, "type", type);
  assert(type == "TensorType");
  H5::readAttribute(group, "name", name);
  // TODO: check link "project"
  H5::readAttribute(group, "dimension", dimension);
  H5::readAttribute(group, "rank", rank);
  H5::readGroup(group, "tensorcomponents",
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
  H5::createAttribute(group, "type", "TensorComponent");
  H5::createAttribute(group, "name", name);
  H5::createHardLink(parent, ".", group, "tensortype");
  H5::createAttribute(group, "indexvalues", indexvalues);
}

TensorComponent::TensorComponent(const H5::CommonFG &loc, const string &entry,
                                 TensorType *tensortype)
    : Common(""), tensortype(tensortype) {
  auto group = loc.openGroup(entry);
  string type;
  H5::readAttribute(group, "type", type);
  assert(type == "TensorComponent");
  H5::readAttribute(group, "name", name);
  // TODO: check link "tensortype"
  H5::readAttribute(group, "indexvalues", indexvalues);
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
  H5::createAttribute(group, "type", "Manifold");
  H5::createAttribute(group, "name", name);
  H5::createHardLink(parent, ".", group, "project");
  H5::createAttribute(group, "dimension", dimension);
#warning "TODO"
  // H5::createGroup(group, "discretizations", discretizations);
}

Manifold::Manifold(const H5::CommonFG &loc, const string &entry,
                   Project *project)
    : Common(""), project(project) {
  auto group = loc.openGroup(entry);
  string type;
  H5::readAttribute(group, "type", type);
  assert(type == "Manifold");
  H5::readAttribute(group, "name", name);
  // TODO: check link "project"
  H5::readAttribute(group, "dimension", dimension);
#warning "TODO"
  // H5::readGroup(group, "discretizations",
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
  H5::createAttribute(group, "type", "TangentSpace");
  H5::createAttribute(group, "name", name);
  H5::createHardLink(parent, ".", group, "project");
  H5::createAttribute(group, "dimension", dimension);
#warning "TODO"
  // H5::createGroup(group, "bases", bases);
}

TangentSpace::TangentSpace(const H5::CommonFG &loc, const string &entry,
                           Project *project)
    : Common(""), project(project) {
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
  H5::createAttribute(group, "type", "Field");
  H5::createAttribute(group, "name", name);
  H5::createHardLink(parent, ".", group, "project");
  H5::createHardLink(parent, string("manifolds/") + manifold->name, group,
                     "manifold");
  H5::createHardLink(parent, string("tangentspaces/") + tangentspace->name,
                     group, "tangentspace");
  H5::createHardLink(parent, string("tensortypes/") + tensortype->name, group,
                     "tensortype");
#warning "TODO"
  // H5::createGroup(group, "discretefields", discretefields);
}

Field::Field(const H5::CommonFG &loc, const string &entry, Project *project)
    : Common(""), project(project) {
  auto group = loc.openGroup(entry);
  string type;
  H5::readAttribute(group, "type", type);
  assert(type == "Field");
  H5::readAttribute(group, "name", name);
  // TODO: check link "project"
  // TODO: Read and interpret objects (shallowly) instead of naively only
  // looking at their names
  {
    auto obj = group.openGroup("manifold");
    string name;
    auto attr = H5::readAttribute(obj, "name", name);
    manifold = project->manifolds.at(name);
  }
  {
    auto obj = group.openGroup("tangentspace");
    string name;
    auto attr = H5::readAttribute(obj, "name", name);
    tangentspace = project->tangentspaces.at(name);
  }
  {
    auto obj = group.openGroup("tensortype");
    string name;
    auto attr = H5::readAttribute(obj, "name", name);
    tensortype = project->tensortypes.at(name);
  }
#warning "TODO"
  // H5::readGroup(group, "discretefields",
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
