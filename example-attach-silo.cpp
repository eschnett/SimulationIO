#include "SimulationIO.hpp"

#include <silo.h>

#include <array>
#include <cmath>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
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
  cout << "example-attach-silo: Create a simple SimulationIO file\n";

  // Project
  auto project = createProject("simulation");

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
      component->createSiloVar(WriteOptions());
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
    // const hsize_t dims[dim] = {nlk, nlj, nli};
    // auto dataspace = H5::DataSpace(dim, dims);
    // auto datatype = H5::getType(double{});
    // Create discrete region
    auto rho_block = discretized_rho->createDiscreteFieldBlock(
        rho->name() + "-" + blocks.at(i)->name(), blocks.at(i));
    auto vel_block = discretized_vel->createDiscreteFieldBlock(
        vel->name() + "-" + blocks.at(i)->name(), blocks.at(i));
    // Create tensor components for this region
    auto scalar3d_component = scalar3d->storage_indices().at(0);
    auto rho_component = rho_block->createDiscreteFieldBlockComponent(
        "scalar", scalar3d_component);
    rho_component->createSiloVar(WriteOptions());
    for (int d = 0; d < dim; ++d) {
      auto vector3d_component = vector3d->storage_indices().at(d);
      auto vel_component = vel_block->createDiscreteFieldBlockComponent(
          dirnames[d], vector3d_component);
      vel_component->createSiloVar(WriteOptions());
    }
  }

  // output
  // cout << *project;

  // Attach data
  for (int pk = 0; pk < npk; ++pk)
    for (int pj = 0; pj < npj; ++pj)
      for (int pi = 0; pi < npi; ++pi) {
        const int p = pi + npi * (pj + npj * pk);
        // const auto lo = point_t(nli * pi, nlj * pj, nlk * pk);
        // const auto hi = lo + point_t(nli, nlj, nlk);
        // const auto box = box_t(lo, hi);
        const auto coordx = make_shared<vector<double>>(npoints);
        const auto coordy = make_shared<vector<double>>(npoints);
        const auto coordz = make_shared<vector<double>>(npoints);
        const auto datarho = make_shared<vector<double>>(npoints);
        const auto datavelx = make_shared<vector<double>>(npoints);
        const auto datavely = make_shared<vector<double>>(npoints);
        const auto datavelz = make_shared<vector<double>>(npoints);
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
              coordx->at(idx) = x;
              coordy->at(idx) = y;
              coordz->at(idx) = z;
              datarho->at(idx) = exp(-0.5 * pow(r, 2));
              datavelx->at(idx) = -y * r * exp(-0.5 * pow(r, 2));
              datavely->at(idx) = +x * r * exp(-0.5 * pow(r, 2));
              datavelz->at(idx) = 0.0;
            }
        // Write coordinates
        for (int d = 0; d < dim; ++d) {
          auto field = coordinates[d]->field();
          auto discretefield = field->discretefields().at(field->name());
          auto block = discretefield->discretefieldblocks().at(
              discretefield->name() + "-" + blocks.at(p)->name());
          auto component = block->discretefieldblockcomponents().at("scalar");
          auto box = block->discretizationblock()->box();
          const auto &coord = d == 0 ? coordx : d == 1 ? coordy : coordz;
          component->silovar()->attachData([=] { return coord->data(); });
        }
        // Write rho
        {
          auto field = rho;
          auto discretefield = field->discretefields().at(field->name());
          auto block = discretefield->discretefieldblocks().at(
              discretefield->name() + "-" + blocks.at(p)->name());
          auto component = block->discretefieldblockcomponents().at("scalar");
          auto box = block->discretizationblock()->box();
          component->silovar()->attachData([=] { return datarho->data(); });
        }
        // Write velocity
        for (int d = 0; d < dim; ++d) {
          auto field = vel;
          auto discretefield = field->discretefields().at(field->name());
          auto block = discretefield->discretefieldblocks().at(
              discretefield->name() + "-" + blocks.at(p)->name());
          auto component =
              block->discretefieldblockcomponents().at(dirnames[d]);
          auto box = block->discretizationblock()->box();
          const auto &datavel =
              d == 0 ? datavelx : d == 1 ? datavely : datavelz;
          component->silovar()->attachData([=] { return datavel->data(); });
        }
      }

  auto filename = "example-silo.silo";
  project->writeSilo(filename);

  cout << "Done.\n";
  return 0;
}
