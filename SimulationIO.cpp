#include "SimulationIO.hpp"

namespace SimulationIO {

// Project

void Project::create_tensortypes() {
  auto s3d = new TensorType("Scalar3D", this, 3, 0);
  new TensorComponent("scalar", s3d, 0, {});

  auto v3d = new TensorType("Vector3D", this, 3, 1);
  new TensorComponent("0", v3d, 0, {0});
  new TensorComponent("1", v3d, 1, {1});
  new TensorComponent("2", v3d, 2, {2});

  auto st3d = new TensorType("SymmetricTensor3D", this, 3, 2);
  new TensorComponent("00", st3d, 0, {0, 0});
  new TensorComponent("01", st3d, 1, {0, 1});
  new TensorComponent("02", st3d, 2, {0, 2});
  new TensorComponent("11", st3d, 3, {1, 1});
  new TensorComponent("12", st3d, 4, {1, 2});
  new TensorComponent("22", st3d, 5, {2, 2});
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

void Project::write(H5::CommonFG &loc) const { assert(0); }

// Tensor types

// TensorType

ostream &TensorType::output(ostream &os, int level) const {
  os << indent(level) << "TensorType \"" << name << "\": dim=" << dimension
     << " rank=" << rank << "\n";
  for (const auto &tc : storedcomponents)
    tc->output(os, level + 1);
  return os;
}

void TensorType::write(H5::CommonFG &loc) const {
  auto group = loc.createGroup(name);
  auto attr_dim =
      group.createAttribute("dimension", h5type(dimension), H5::DataSpace());
  attr_dim.write(h5type(dimension), &dimension);
  auto attr_rank = group.createAttribute("rank", h5type(rank), H5::DataSpace());
  attr_rank.write(h5type(rank), &rank);
  auto sc_group = group.createGroup("storedcomponents");
  for (const auto &tc : storedcomponents)
    tc->write(sc_group);
}

TensorType::TensorType(const string &name, Project *project,
                       const H5::H5Location &loc)
    : Common(name), project(project) {
  auto attr_dim = loc.openAttribute("dimension");
  attr_dim.read(h5type(dimension), &dimension);
  auto attr_rank = loc.openAttribute("rank");
  attr_rank.read(h5type(rank), &rank);
#warning "read storedcomponents"
}

// TensorComponent

ostream &TensorComponent::output(ostream &os, int level) const {
  os << indent(level) << "TensorComponent \"" << name << "\": tensortype=\""
     << tensortype->name << "\" storedcomponent=" << storedcomponent
     << " indices=[";
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
  auto attr_sc = group.createAttribute("storedcomponent",
                                       H5::IntType(H5::PredType::NATIVE_INT),
                                       H5::DataSpace());
  attr_sc.write(H5::IntType(H5::PredType::NATIVE_INT), &storedcomponent);
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
