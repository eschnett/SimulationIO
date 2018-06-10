#include "SimulationIO.hpp"

#include <asdf.hpp>

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
  cout << "example-asdf: Create a simple SimulationIO file\n";

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

  // Fields
  array<shared_ptr<Field>, dim> coordinates;
  for (int d = 0; d < dim; ++d)
    coordinates[d] = project->createField(dirnames[d], configuration, manifold,
                                          tangentspace, scalar3d);
  auto rho = project->createField("rho", configuration, manifold, tangentspace,
                                  scalar3d);
  auto vel = project->createField("vel", configuration, manifold, tangentspace,
                                  vector3d);

  array<shared_ptr<DiscreteField>, dim> discretized_coordinates;
  for (int d = 0; d < dim; ++d)
    discretized_coordinates[d] = coordinates[d]->createDiscreteField(
        coordinates[d]->name(), configuration, discretization, basis);
  auto discretized_rho = rho->createDiscreteField(rho->name(), configuration,
                                                  discretization, basis);
  auto discretized_vel = vel->createDiscreteField(vel->name(), configuration,
                                                  discretization, basis);

  // CoordinateSystem
  auto coordinatesystem =
      project->createCoordinateSystem("Cartesian", configuration, manifold);
  array<shared_ptr<CoordinateField>, dim> coordinatefields;
  for (int d = 0; d < dim; ++d)
    coordinatefields[d] =
        coordinatesystem->createCoordinateField(dirnames[d], d, coordinates[d]);

  // DiscretizationBlocks
  for (int pk = 0; pk < npk; ++pk)
    for (int pj = 0; pj < npj; ++pj)
      for (int pi = 0; pi < npi; ++pi) {
        const int p = pi + npi * (pj + npj * pk);

        // Coordinates
        array<double, dim> origin;
        getcoords(0, 0, 0, origin[0], origin[1], origin[2]);
        array<double, dim> first;
        getcoords(1, 1, 1, first[0], first[1], first[2]);
        array<vector<double>, dim> delta;
        for (int d = 0; d < dim; ++d) {
          delta[d] = {0, 0, 0};
          delta[d][d] = first[d] - origin[d];
        }
        for (int d = 0; d < dim; ++d) {
          auto block = discretized_coordinates[d]->createDiscreteFieldBlock(
              discretized_coordinates[d]->name() + "-" + blocks.at(p)->name(),
              blocks.at(p));
          auto scalar3d_component = scalar3d->storage_indices().at(0);
          auto coordinate_component = block->createDiscreteFieldBlockComponent(
              "scalar", scalar3d_component);
          coordinate_component->createDataRange(origin[d], delta[d]);
        }

        // Fields

        const auto calcblock{
            [=](const function<double(double, double, double, double)> &f) {
              vector<double> res(npoints);
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
                    res.at(idx) = f(x, y, z, r);
                  }
              return make_constant_memoized(shared_ptr<ASDF::block_t>(
                  make_shared<ASDF::typed_block_t<double>>(move(res))));
            }};

        const ASDF::memoized<ASDF::block_t> mrho{
            calcblock([](double x, double y, double z, double r) {
              return exp(-0.5 * pow(r, 2));
            })};
        const array<ASDF::memoized<ASDF::block_t>, dim> mvel{
            calcblock([](double x, double y, double z, double r) {
              return -y * r * exp(-0.5 * pow(r, 2));
            }),
            calcblock([](double x, double y, double z, double r) {
              return +x * r * exp(-0.5 * pow(r, 2));
            }),
            calcblock(
                [](double x, double y, double z, double r) { return 0.0; })};

        auto rho_block = discretized_rho->createDiscreteFieldBlock(
            rho->name() + "-" + blocks.at(p)->name(), blocks.at(p));
        auto vel_block = discretized_vel->createDiscreteFieldBlock(
            vel->name() + "-" + blocks.at(p)->name(), blocks.at(p));
        // Create tensor components for this region
        auto scalar3d_component = scalar3d->storage_indices().at(0);
        auto rho_component = rho_block->createDiscreteFieldBlockComponent(
            "scalar", scalar3d_component);
        rho_component->createASDFData<double>(mrho);
        for (int d = 0; d < dim; ++d) {
          auto vector3d_component = vector3d->storage_indices().at(d);
          auto vel_component = vel_block->createDiscreteFieldBlockComponent(
              dirnames[d], vector3d_component);
          vel_component->createASDFData<double>(mvel[d]);
        }
      }

  // output
  // cout << *project;

  auto filename = "example.asdf";
  ofstream file(filename, ios::binary | ios::trunc | ios::out);
  project->writeASDF(file);
  file.close();

  cout << "Done.\n";
  return 0;
}
