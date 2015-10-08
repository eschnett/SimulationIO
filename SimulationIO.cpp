#include "SimulationIO.hpp"

namespace SimulationIO {

// Tensor types

map<string, TensorType *> tensortypes;

ostream &TensorType::output(ostream &os, int level) const {
  os << indent(level) << "TensorType \"" << name << "\": dim=" << dimension
     << " rank=" << rank << "\n";
  for (const auto &tc : storedcomponents)
    tc->output(os, level + 1);
  return os;
}

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

namespace detail {
auto S3D = new TensorType("Scalar3D", 3, 0);
auto S3D_scalar = new TensorComponent("scalar", S3D, 0, {});

auto V3D = new TensorType("Vector3D", 3, 1);
auto V3D_0 = new TensorComponent("0", V3D, 0, {0});
auto V3D_1 = new TensorComponent("1", V3D, 1, {1});
auto V3D_2 = new TensorComponent("2", V3D, 2, {2});

auto ST3D = new TensorType("SymmetricTensor3D", 3, 2);
auto ST3D_00 = new TensorComponent("00", ST3D, 0, {0, 0});
auto ST3D_01 = new TensorComponent("01", ST3D, 1, {0, 1});
auto ST3D_02 = new TensorComponent("02", ST3D, 2, {0, 2});
auto ST3D_11 = new TensorComponent("11", ST3D, 3, {1, 1});
auto ST3D_12 = new TensorComponent("12", ST3D, 4, {1, 2});
auto ST3D_22 = new TensorComponent("22", ST3D, 5, {2, 2});
}

// High-level continuum concepts

map<string, Manifold *> manifolds;
map<string, TangentSpace *> tangentspaces;
map<string, Field *> fields;

// Coordinates

map<string, CoordinateSystem *> coordinates;
}
