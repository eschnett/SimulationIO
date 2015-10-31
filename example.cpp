#include "SimulationIO.hpp"

#include <cmath>
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

const int dim = 3;
const char *const dirnames[] = {"x", "y", "z"};

const int nli = 10, nlj = 10, nlk = 10;
const int npoints = nli * nlj * nlk;
const int npi = 4, npj = 4, npk = 4;
const int ngrids = npi * npj * npk;
const int ni = npi * nli, nj = npj * nlj, nk = npk * nlk;
const double xmin = -1.0, ymin = -1.0, zmin = -1.0;
const double xmax = 1.0, ymax = 1.0, zmax = 1.0;
inline void getcoords(int i, int j, int k, double &x, double &y, double &z) {
  assert(i >= 0 && i < ni && j >= 0 && j < nj && k >= 0 && k < nk);
  x = xmin + (i + 0.5) * (xmax - xmin) / ni;
  y = ymin + (j + 0.5) * (ymax - ymin) / nj;
  z = zmin + (k + 0.5) * (zmax - zmin) / nk;
}

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
  auto manifold = project->createManifold("domain", configuration, dim);
  auto tangentspace = project->createTangentSpace("space", configuration, dim);

  // Discretization for Manifold
  auto discretization =
      manifold->createDiscretization("uniform", configuration);
  vector<shared_ptr<DiscretizationBlock>> blocks;
  for (int p = 0; p < ngrids; ++p) {
    ostringstream name;
    name << "grid." << p;
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
    for (int p = 0; p < ngrids; ++p) {
      auto block = discretefield->createDiscreteFieldBlock(
          discretefield->name + "-" + blocks.at(p)->name, blocks.at(p));
      const auto &scalar3d_component = scalar3d->storage_indices.at(0);
      auto component = block->createDiscreteFieldBlockComponent(
          "scalar", scalar3d_component);
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
      rho->createDiscreteField(rho->name, configuration, discretization, basis);
  auto discretized_vel =
      vel->createDiscreteField(vel->name, configuration, discretization, basis);
  for (int i = 0; i < ngrids; ++i) {
    // Create discrete region
    auto rho_block = discretized_rho->createDiscreteFieldBlock(
        rho->name + "-" + blocks.at(i)->name, blocks.at(i));
    auto vel_block = discretized_vel->createDiscreteFieldBlock(
        vel->name + "-" + blocks.at(i)->name, blocks.at(i));
    // Create tensor components for this region
    const auto &scalar3d_component = scalar3d->storage_indices.at(0);
    auto rho_component = rho_block->createDiscreteFieldBlockComponent(
        "scalar", scalar3d_component);
    // TODO: write data
    (void)rho_component;
    for (int d = 0; d < dim; ++d) {
      const auto &vector3d_component = vector3d->storage_indices.at(d);
      auto vel_component = vel_block->createDiscreteFieldBlockComponent(
          dirnames[d], vector3d_component);
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

  // Write data
  for (int pk = 0; pk < npk; ++pk)
    for (int pj = 0; pj < npj; ++pj)
      for (int pi = 0; pi < npi; ++pi) {
        const int p = pi + npi * (pj + npj * pk);
        vector<double> coordx(npoints), coordy(npoints), coordz(npoints),
            datarho(npoints), datavelx(npoints), datavely(npoints),
            datavelz(npoints);
        for (int lk = 0; lk < nlk; ++lk)
          for (int lj = 0; lj < nlj; ++lj)
            for (int li = 0; li < nli; ++li) {
              const int idx = li + nli * (lj + nlj * lk);
              const int i = li + nli * pi;
              const int j = lj + nlj * pj;
              const int k = lk + nlk * pk;
              double x, y, z;
              getcoords(i, j, k, x, y, z);
              const double r = sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
              coordx.at(idx) = x;
              coordy.at(idx) = y;
              coordz.at(idx) = z;
              datarho.at(idx) = exp(-0.5 * pow(r, 2));
              datavelx.at(idx) = -y * r * exp(-0.5 * pow(r, 2));
              datavely.at(idx) = +x * r * exp(-0.5 * pow(r, 2));
              datavelz.at(idx) = 0.0;
            }
        // Write coordinates
        for (int d = 0; d < dim; ++d) {
          const auto &field = coordinates[d]->field;
          const auto &discretefield = field->discretefields.at(field->name);
          const auto &block = discretefield->discretefieldblocks.at(
              discretefield->name + "-" + blocks.at(p)->name);
          const auto &component =
              block->discretefieldblockcomponents.at("scalar");
          const auto &path = component->getPath();
          auto group = file.openGroup(path);
          const auto &name = component->getName();
          const hsize_t dims[dim] = {nlk, nlj, nli};
          auto dataspace = H5::DataSpace(dim, dims, NULL);
          auto datatype = H5::getType(double());
          auto dataset = group.createDataSet(name, datatype, dataspace);
          const auto &data = d == 0 ? coordx : d == 1 ? coordy : coordz;
          dataset.write(data.data(), H5::getType(data[0]));
        }
        // Write rho
        {
          const auto &field = rho;
          const auto &discretefield = field->discretefields.at(field->name);
          const auto &block = discretefield->discretefieldblocks.at(
              discretefield->name + "-" + blocks.at(p)->name);
          const auto &component =
              block->discretefieldblockcomponents.at("scalar");
          const auto &path = component->getPath();
          auto group = file.openGroup(path);
          const auto &name = component->getName();
          const hsize_t dims[dim] = {nlk, nlj, nli};
          auto dataspace = H5::DataSpace(dim, dims, NULL);
          auto datatype = H5::getType(double());
          auto dataset = group.createDataSet(name, datatype, dataspace);
          const auto &data = datarho;
          dataset.write(data.data(), H5::getType(data[0]));
        }
        // Write velocity
        for (int d = 0; d < dim; ++d) {
          const auto &field = vel;
          const auto &discretefield = field->discretefields.at(field->name);
          const auto &block = discretefield->discretefieldblocks.at(
              discretefield->name + "-" + blocks.at(p)->name);
          const auto &component =
              block->discretefieldblockcomponents.at(dirnames[d]);
          const auto &path = component->getPath();
          auto group = file.openGroup(path);
          const auto &name = component->getName();
          const hsize_t dims[dim] = {nlk, nlj, nli};
          auto dataspace = H5::DataSpace(dim, dims, NULL);
          auto datatype = H5::getType(double());
          auto dataset = group.createDataSet(name, datatype, dataspace);
          const auto &data = d == 0 ? datavelx : d == 1 ? datavely : datavelz;
          dataset.write(data.data(), H5::getType(data[0]));
        }
      }

  return 0;
}
