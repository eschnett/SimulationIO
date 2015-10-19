#include "SimulationIO.hpp"

#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

using namespace SimulationIO;

using std::cout;
using std::ostringstream;
using std::make_shared;
using std::shared_ptr;
using std::string;
using std::vector;

const char *const dirnames[] = {"x", "y", "z"};

int main(int argc, char **argv) {

  {
    auto x = make_shared<int>(1);
    shared_ptr<int> y = x;
    x.reset();
  }

  // Project
  auto project = createProject("simulation");

  // Configuration
  auto configuration = project->createConfiguration("global");

  // TensorTypes
  project->createStandardTensortypes();
  auto scalar3d = project->tensortypes.at("Scalar3D");
  auto vector3d = project->tensortypes.at("Vector3D");

  // Manifold and TangentSpace, both 3D
  const int dim = 3;
  auto manifold = project->createManifold("domain", dim);
  auto tangentspace = project->createTangentSpace("space", dim);

  // Discretization for Manifold
  auto discretization = manifold->createDiscretization("uniform");
  const int ngrids = 10;
  vector<DiscretizationBlock *> blocks;
  for (int i = 0; i < ngrids; ++i) {
    ostringstream name;
    name << "grid." << i;
    blocks.push_back(discretization->createDiscretizationBlock(name.str()));
  }

  // Basis for TangentSpace
  auto basis = tangentspace->createBasis("Cartesian");
  vector<BasisVector *> directions;
  for (int d = 0; d < dim; ++d) {
    directions.push_back(basis->createBasisVector(dirnames[d], d));
  }

  // Field
  auto rho = project->createField("rho", manifold, tangentspace, scalar3d);
  auto vel = project->createField("vel", manifold, tangentspace, vector3d);
  auto discretized_rho = rho->createDiscreteField("rho", discretization, basis);
  auto discretized_vel = vel->createDiscreteField("vel", discretization, basis);
  for (int i = 0; i < ngrids; ++i) {
    // Create discrete region
    auto rho_block = discretized_rho->createDiscreteFieldBlock(
        rho->name + "-" + blocks.at(i)->name, blocks.at(i));
    auto vel_block = discretized_vel->createDiscreteFieldBlock(
        vel->name + "-" + blocks.at(i)->name, blocks.at(i));
    // Create tensor components for this region
    auto scalar3d_component = scalar3d->storage_indices.at(0);
    auto rho_component = rho_block->createDiscreteFieldBlockData(
        rho_block->name, scalar3d_component);
    (void)rho_component;
    for (int d = 0; d < dim; ++d) {
      auto vector3d_component = vector3d->storage_indices.at(d);
      auto vel_component = vel_block->createDiscreteFieldBlockData(
          vel_block->name + "-" + dirnames[d], vector3d_component);
      (void)vel_component;
    }
  }

  // output
  cout << *project;

  // Write file
  auto filename = "example.h5";
  auto file = H5::H5File(filename, H5F_ACC_TRUNC);
  project->write(file);

  return 0;
}
