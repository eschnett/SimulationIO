#include "SimulationIO.hpp"

#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

using namespace SimulationIO;

using std::cout;
using std::ostringstream;
using std::shared_ptr;
using std::string;
using std::vector;

const char *const dirnames[] = {"x", "y", "z"};

int main(int argc, char **argv) {

  // Project
  auto project = createProject("simulation");

  // Configuration
  auto configuration = project->createConfiguration("global");

  // TensorTypes
  project->createStandardTensorTypes();
  auto scalar3d = project->tensortypes.at("Scalar3D");
  auto vector3d = project->tensortypes.at("Vector3D");

  // Manifold and TangentSpace, both 3D
  const int dim = 3;
  auto manifold = project->createManifold("domain", configuration, dim);
  auto tangentspace = project->createTangentSpace("space", configuration, dim);

  // Discretization for Manifold
  auto discretization =
      manifold->createDiscretization("uniform", configuration);
  const int ngrids = 10;
  vector<shared_ptr<DiscretizationBlock>> blocks;
  for (int i = 0; i < ngrids; ++i) {
    ostringstream name;
    name << "grid." << i;
    blocks.push_back(discretization->createDiscretizationBlock(name.str()));
  }

  // Basis for TangentSpace
  auto basis = tangentspace->createBasis("Cartesian", configuration);
  vector<shared_ptr<BasisVector>> directions;
  for (int d = 0; d < dim; ++d) {
    directions.push_back(basis->createBasisVector(dirnames[d], d));
  }

  // Coordinate System
  auto coordinatesystem =
      project->createCoordinateSystem("Cartesian", configuration, manifold);
  vector<shared_ptr<CoordinateField>> coordinates;
  for (int d = 0; d < dim; ++d) {
    auto field = project->createField(dirnames[d], configuration, manifold,
                                      tangentspace, scalar3d);
    auto discretefield = field->createDiscreteField(field->name, configuration,
                                                    discretization, basis);
    for (int i = 0; i < ngrids; ++i) {
      auto block = discretefield->createDiscreteFieldBlock(
          discretefield->name + "-" + blocks.at(i)->name, blocks.at(i));
      const auto &scalar3d_component = scalar3d->storage_indices.at(0);
      auto component = block->createDiscreteFieldBlockComponent(
          block->name, scalar3d_component);
      // TODO: Write coordinate information
      (void)component;
    }
    coordinates.push_back(
        coordinatesystem->createCoordinateField(dirnames[d], d, field));
  }

  // Fields
  auto rho = project->createField("rho", configuration, manifold, tangentspace,
                                  scalar3d);
  auto vel = project->createField("vel", configuration, manifold, tangentspace,
                                  vector3d);
  auto discretized_rho =
      rho->createDiscreteField("rho", configuration, discretization, basis);
  auto discretized_vel =
      vel->createDiscreteField("vel", configuration, discretization, basis);
  for (int i = 0; i < ngrids; ++i) {
    // Create discrete region
    auto rho_block = discretized_rho->createDiscreteFieldBlock(
        rho->name + "-" + blocks.at(i)->name, blocks.at(i));
    auto vel_block = discretized_vel->createDiscreteFieldBlock(
        vel->name + "-" + blocks.at(i)->name, blocks.at(i));
    // Create tensor components for this region
    const auto &scalar3d_component = scalar3d->storage_indices.at(0);
    auto rho_component = rho_block->createDiscreteFieldBlockComponent(
        rho_block->name, scalar3d_component);
    // TODO: write data
    (void)rho_component;
    for (int d = 0; d < dim; ++d) {
      const auto &vector3d_component = vector3d->storage_indices.at(d);
      auto vel_component = vel_block->createDiscreteFieldBlockComponent(
          vel_block->name + "-" + dirnames[d], vector3d_component);
      // TODO: write data
      (void)vel_component;
    }
  }

  // output
  cout << *project;

  // Write file
  auto filename = "example.h5";
  auto fapl = H5::FileAccPropList();
  fapl.setLibverBounds(H5F_LIBVER_LATEST, H5F_LIBVER_LATEST);
  auto file =
      H5::H5File(filename, H5F_ACC_TRUNC, H5::FileCreatPropList::DEFAULT, fapl);
  project->write(file);

  return 0;
}
