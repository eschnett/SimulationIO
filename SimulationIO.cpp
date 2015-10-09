#include "SimulationIO.hpp"

namespace SimulationIO {

// Project

void Project::create_standard_tensortypes() {
  auto s3d = new TensorType("Scalar3D", this, 3, 0);
  new TensorComponent("scalar", s3d, (vector<int>){});

  auto v3d = new TensorType("Vector3D", this, 3, 1);
  new TensorComponent("0", v3d, {0});
  new TensorComponent("1", v3d, {1});
  new TensorComponent("2", v3d, {2});

  auto st3d = new TensorType("SymmetricTensor3D", this, 3, 2);
  new TensorComponent("00", st3d, {0, 0});
  new TensorComponent("01", st3d, {0, 1});
  new TensorComponent("02", st3d, {0, 2});
  new TensorComponent("11", st3d, {1, 1});
  new TensorComponent("12", st3d, {1, 2});
  new TensorComponent("22", st3d, {2, 2});
}

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
  H5_create_group(loc, "tensortypes", tensortypes);
  H5_create_group(loc, "manifolds", manifolds);
  H5_create_group(loc, "tangentspaces", tangentspaces);
  H5_create_group(loc, "fields", fields);
#warning "TODO"
  // H5_create_group(loc, "coordiantesystems", coordinatesystems);
}

Project::Project(const string &name, const H5::CommonFG &loc) : Common(name) {
  assert(invariant());
  H5_read_group(loc, "tensortypes", this, tensortypes);
#warning "TODO"
  // H5_read_group(loc, "manifolds", this, manifolds);
  // H5_read_group(loc, "tangentspaces", this, tangentspaces);
  // H5_read_group(loc, "fields", this, fields);
  // H5_read_group(loc, "coordinatesystems", this, coordinatesystems);
}

// Tensor types

// TensorType

ostream &TensorType::output(ostream &os, int level) const {
  os << indent(level) << "TensorType \"" << name << "\": dim=" << dimension
     << " rank=" << rank << "\n";
  for (const auto &tc : tensorcomponents)
    tc.second->output(os, level + 1);
  return os;
}

void TensorType::write(H5::CommonFG &loc) const {
  auto group = loc.createGroup(name);
  H5_create_attribute(group, "dimension", dimension);
  H5_create_attribute(group, "rank", rank);
  H5_create_group(group, "tensorcomponents", tensorcomponents);
}

TensorType::TensorType(const string &name, Project *project,
                       const H5::CommonFG &loc)
    : Common(name), project(project) {
  auto group = loc.openGroup(name);
  H5_read_attribute(group, "dimension", dimension);
  H5_read_attribute(group, "rank", rank);
  project->insert(name, this);
  assert(invariant());
  H5_read_group(group, "tensorcomponents", this, tensorcomponents);
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
  H5_create_attribute(group, "indexvalues", indexvalues);
}

TensorComponent::TensorComponent(const string &name, TensorType *tensortype,
                                 const H5::CommonFG &loc)
    : Common(name), tensortype(tensortype) {
  auto group = loc.openGroup(name);
  H5_read_attribute(group, "indexvalues", indexvalues);
  tensortype->insert(name, this);
  assert(invariant());
}

// High-level continuum concepts

// Manifold

ostream &Manifold::output(ostream &os, int level) const { assert(0); }
void Manifold::write(H5::CommonFG &loc) const { assert(0); }

// TangentSpace

ostream &TangentSpace::output(ostream &os, int level) const { assert(0); }
void TangentSpace::write(H5::CommonFG &loc) const { assert(0); }

// Field

ostream &Field::output(ostream &os, int level) const { assert(0); }
void Field::write(H5::CommonFG &loc) const { assert(0); }

// Coordinates
}
