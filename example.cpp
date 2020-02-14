#include "SimulationIO.hpp"

#include <H5Cpp.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

using namespace SimulationIO;
using namespace std;

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
  cout << "example: Create a simple SimulationIO file\n";

  // Project
  auto project = createProject("simulation");

#if 0
  auto databuffer = make_shared<DataBuffer>(dim, H5::getType(double{}));
#endif

  // Configuration
  auto configuration = project->createConfiguration("global");

  // TensorTypes
  project->createStandardTensorTypes();
  auto scalar3d = project->tensortypes().at("Scalar3D");
  auto vector3d = project->tensortypes().at("Vector3D");

  // Manifold and TangentSpace, both 3D
  auto manifold = project->createManifold("domain", configuration, dim);
  auto tangentspace =
      project->createTangentSpace("tangentspace", configuration, dim);

  // Discretization for Manifold
  auto discretization =
      manifold->createDiscretization("uniform", configuration);
  vector<shared_ptr<DiscretizationBlock>> blocks;
  for (int pk = 0; pk < npk; ++pk)
    for (int pj = 0; pj < npj; ++pj)
      for (int pi = 0; pi < npi; ++pi) {
        const int p = pi + npi * (pj + npj * pk);
        ostringstream name;
        name << "grid." << p;
        auto block = discretization->createDiscretizationBlock(name.str());
        vector<long long> offset{nli * pi, nlj * pj, nlk * pk};
        vector<long long> shape{nli, nlj, nlk};
        block->setBox(box_t(offset, point_t(offset) + shape));
        block->setActive(region_t(box_t(offset, point_t(offset) + shape)));
        blocks.push_back(block);
      }

  // Basis for TangentSpace
  auto basis = tangentspace->createBasis("Cartesian", configuration);
  vector<shared_ptr<BasisVector>> directions;
  for (int d = 0; d < dim; ++d) {
    directions.push_back(basis->createBasisVector(dirnames[d], d));
  }

  WriteOptions write_options;
  write_options.compress = true;
  write_options.compression_method = WriteOptions::compression_method_t::zlib;
  write_options.compression_level = 9;
  write_options.shuffle = true;
  write_options.checksum = true;

  // Coordinate System
  auto coordinatesystem =
      project->createCoordinateSystem("Cartesian", configuration, manifold);
  vector<shared_ptr<CoordinateField>> coordinates;
  for (int d = 0; d < dim; ++d) {
    auto field = project->createField(dirnames[d], configuration, manifold,
                                      tangentspace, scalar3d);
    auto discretefield = field->createDiscreteField(
        field->name(), configuration, discretization, basis);
    for (int p = 0; p < ngrids; ++p) {
      auto block = discretefield->createDiscreteFieldBlock(
          discretefield->name() + "-" + blocks.at(p)->name(), blocks.at(p));
      auto scalar3d_component = scalar3d->storage_indices().at(0);
      auto component = block->createDiscreteFieldBlockComponent(
          "scalar", scalar3d_component);

      array<double, dim> origin;
      getcoords(0, 0, 0, origin[0], origin[1], origin[2]);
      array<double, dim> first;
      getcoords(1, 1, 1, first[0], first[1], first[2]);
      array<vector<double>, dim> delta;
      for (int d = 0; d < dim; ++d) {
        delta[d] = {0, 0, 0};
        delta[d][d] = first[d] - origin[d];
      }
      component->createDataRange(write_options, origin[d], delta[d]);
    }
    coordinates.push_back(
        coordinatesystem->createCoordinateField(dirnames[d], d, field));
  }

  // Fields
  auto rho = project->createField("rho", configuration, manifold, tangentspace,
                                  scalar3d);
  auto vel = project->createField("vel", configuration, manifold, tangentspace,
                                  vector3d);
  auto discretized_rho = rho->createDiscreteField(rho->name(), configuration,
                                                  discretization, basis);
  auto discretized_vel = vel->createDiscreteField(vel->name(), configuration,
                                                  discretization, basis);
  for (int i = 0; i < ngrids; ++i) {
    // Create discrete region
    auto rho_block = discretized_rho->createDiscreteFieldBlock(
        rho->name() + "-" + blocks.at(i)->name(), blocks.at(i));
    auto vel_block = discretized_vel->createDiscreteFieldBlock(
        vel->name() + "-" + blocks.at(i)->name(), blocks.at(i));
    // Create tensor components for this region
    auto scalar3d_component = scalar3d->storage_indices().at(0);
    auto rho_component = rho_block->createDiscreteFieldBlockComponent(
        "scalar", scalar3d_component);
#if 1
    auto rho_dataset = rho_component->createDataSet(write_options);
#else
    auto rho_dataset = rho_component->createDataBufferEntry(
        write_options, H5::getType(double{}), databuffer);
#endif
    for (int d = 0; d < dim; ++d) {
      auto vector3d_component = vector3d->storage_indices().at(d);
      auto vel_component = vel_block->createDiscreteFieldBlockComponent(
          dirnames[d], vector3d_component);
      auto vel_dataset = vel_component->createDataSet(write_options);
    }
  }

  // output
  // cout << *project;

  // Write file
  auto filename = "example.s5";
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

        // Set data
        vector<double> datarho(npoints);
        array<vector<double>, dim> datavel{vector<double>(npoints),
                                           vector<double>(npoints),
                                           vector<double>(npoints)};
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
              datarho.at(idx) = exp(-0.5 * pow(r, 2));
              datavel[0].at(idx) = -y * r * exp(-0.5 * pow(r, 2));
              datavel[1].at(idx) = +x * r * exp(-0.5 * pow(r, 2));
              datavel[2].at(idx) = 0.0;
            }

        // Write rho
        {
          auto field = rho;
          auto discretefield = field->discretefields().at(field->name());
          auto block = discretefield->discretefieldblocks().at(
              discretefield->name() + "-" + blocks.at(p)->name());
          auto component = block->discretefieldblockcomponents().at("scalar");
          component->dataset()->writeData(datarho);
        }
        // Write velocity
        for (int d = 0; d < dim; ++d) {
          auto field = vel;
          auto discretefield = field->discretefields().at(field->name());
          auto block = discretefield->discretefieldblocks().at(
              discretefield->name() + "-" + blocks.at(p)->name());
          auto component =
              block->discretefieldblockcomponents().at(dirnames[d]);
          component->dataset()->writeData(datavel[d]);
        }
      }

  return 0;
}
