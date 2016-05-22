#include "SimulationIO.hpp"

#include <chrono>
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

  std::chrono::time_point<std::chrono::system_clock> start, end;
  const auto t0 = std::chrono::system_clock::now();

  // Project
  auto project = createProject("simulation");

  // Configuration
  auto configuration = project->createConfiguration("global");

  // TensorTypes
  project->createStandardTensorTypes();
  auto scalar3d = project->tensortypes().at("Scalar3D");
  auto vector3d = project->tensortypes().at("Vector3D");

  // Manifold and TangentSpace, both 3D
  const int dim = 3;
  auto manifold = project->createManifold("domain", configuration, dim);
  auto tangentspace = project->createTangentSpace("space", configuration, dim);

  // Discretization for Manifold
  auto discretization =
      manifold->createDiscretization("uniform", configuration);
  const int ngrids = 100;
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

  // Fields
  const int nfields = 100;
  vector<shared_ptr<Field>> fields;
  vector<shared_ptr<DiscreteField>> discretefields;
  for (int f = 0; f < nfields; ++f) {
    ostringstream name;
    name << "field." << f;
    auto field = project->createField(name.str(), configuration, manifold,
                                      tangentspace, vector3d);
    fields.push_back(field);
    auto discretefield = field->createDiscreteField(
        field->name(), configuration, discretization, basis);
    discretefields.push_back(discretefield);
  }
  for (int i = 0; i < ngrids; ++i) {
    // Create discrete regions
    for (int f = 0; f < nfields; ++f) {
      auto discretefield = discretefields.at(f);
      auto field_block = discretefield->createDiscreteFieldBlock(
          discretefield->name() + "-" + blocks.at(i)->name(), blocks.at(i));
      for (int d = 0; d < dim; ++d) {
        auto vector3d_component = vector3d->storage_indices().at(d);
        auto field_component = field_block->createDiscreteFieldBlockComponent(
            field_block->name() + "-" + dirnames[d], vector3d_component);
        (void)field_component;
      }
    }
  }

  const auto t1 = std::chrono::system_clock::now();

  // Write file
  auto filename = "benchmark.s5";
  {
    auto fapl = H5::FileAccPropList();
    fapl.setLibverBounds(H5F_LIBVER_LATEST, H5F_LIBVER_LATEST);
    auto file = H5::H5File(filename, H5F_ACC_TRUNC,
                           H5::FileCreatPropList::DEFAULT, fapl);
    project->write(file);
  }

  const auto t2 = std::chrono::system_clock::now();

  // Read file
  {
    auto file = H5::H5File(filename, H5F_ACC_RDONLY);
    auto project2 = readProject(file);
  }

  const auto t3 = std::chrono::system_clock::now();

  const std::chrono::duration<double> time_create = t1 - t0;
  const std::chrono::duration<double> time_write = t2 - t1;
  const std::chrono::duration<double> time_read = t3 - t2;
  cout << "Create time: " << time_create.count() << "\n"
       << "Write time: " << time_write.count() << "\n"
       << "Read time: " << time_read.count() << "\n";

  return 0;
}
