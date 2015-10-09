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

void Project::write(H5::CommonFG &loc) const { assert(0); }

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
  auto attr_dim =
      group.createAttribute("dimension", h5type(dimension), H5::DataSpace());
  attr_dim.write(h5type(dimension), &dimension);
  auto attr_rank = group.createAttribute("rank", h5type(rank), H5::DataSpace());
  attr_rank.write(h5type(rank), &rank);
  auto tc_group = group.createGroup("tensorcomponents");
  for (const auto &tc : tensorcomponents)
    tc.second->write(tc_group);
}

TensorType::TensorType(const string &name, Project *project,
                       const H5::CommonFG &loc)
    : Common(name), project(project) {
  auto group = loc.openGroup(name);
  auto attr_dim = group.openAttribute("dimension");
  auto attr_dim_space = attr_dim.getSpace();
  assert(attr_dim_space.getSimpleExtentType() == H5S_SCALAR);
  attr_dim.read(h5type(dimension), &dimension);
  auto attr_rank = group.openAttribute("rank");
  auto attr_rank_space = attr_rank.getSpace();
  assert(attr_rank_space.getSimpleExtentType() == H5S_SCALAR);
  attr_rank.read(h5type(rank), &rank);
  project->insert(name, this);
  assert(invariant());
  auto tc_group = group.openGroup("tensorcomponents");
  hsize_t idx = 0;
  H5_iterate(tc_group, H5_INDEX_NAME, H5_ITER_NATIVE, &idx,
             [&](H5::Group group, const string &name, const H5L_info_t *info) {
               new TensorComponent(name, this, group);
               return 0;
             });
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
  hsize_t dims[1] = {indexvalues.size()};
  auto attr_ivs = group.createAttribute(
      "indexvalues", h5type(*indexvalues.data()), H5::DataSpace(1, dims));
  const void *buf = indexvalues.data();
  int dummy;
  if (!buf)
    buf = &dummy; // HDF5 is overly cautious
  attr_ivs.write(h5type(*indexvalues.data()), buf);
}

TensorComponent::TensorComponent(const string &name, TensorType *tensortype,
                                 const H5::CommonFG &loc)
    : Common(name), tensortype(tensortype) {
  auto group = loc.openGroup(name);
  auto attr_ivs = group.openAttribute("indexvalues");
  auto attr_ivs_space = attr_ivs.getSpace();
  auto npoints = attr_ivs_space.getSimpleExtentNpoints();
  indexvalues.resize(npoints);
  void *buf = indexvalues.data();
  int dummy;
  if (!buf)
    buf = &dummy; // HDF5 is overly cautious
  attr_ivs.read(h5type(*indexvalues.data()), buf);
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
