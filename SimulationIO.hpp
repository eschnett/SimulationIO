#ifndef SIMULATIONIO_HPP
#define SIMULATIONIO_HPP

// Note: Unless noted otherwise, all pointers are non-null

// When translating to HDF5:
// - structs become objects, usually groups
// - simple struct fields (int, string) become attributes of the group
// - pointers become links inside the group
// - sets containing non-pointers become objects inside the group
// - sets containing pointers become links inside a subgroup of the group
// - vectors of simple types (int, string) become attributes
// - other vectors become objects inside a subgroup the group, sorted
//   alphabetically

#include <hdf5.h>

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace SimulationIO {

// using std::lrint;
using std::pow;
using std::set;
using std::shared_ptr;
using std::string;
using std::vector;

namespace {
inline int ipow(int base, int exp) {
  if (exp == 0)
    return 1;
  return lrint(pow(base, exp));
}
}

// Tensor types

struct TensorType;
struct TensorComponent;

set<TensorType *> tensortypes;

struct TensorType {
  string name;
  int dimension;
  int rank;
  vector<TensorComponent *> storedcomponents;
  bool invariant() const {
    return dimension >= 0 && rank >= 0 &&
           int(storedcomponents.size()) <= ipow(dimension, rank);
  }
  TensorType(const string &name, int dimension, int rank)
      : name(name), dimension(dimension), rank(rank) {
    assert(invariant());
    tensortypes.insert(this);
  }
  void push_back(TensorComponent *tensorcomponent) {
    storedcomponents.push_back(tensorcomponent);
  }
};

struct TensorComponent {
  string name;
  TensorType *tensortype;
  // We use objects to denote most concepts, but we make an exception
  // for tensor component indices and tangent space basis vectors,
  // which we number consecutively starting from zero. This simplifies
  // the representation, and it introduces a canonical order (e.g. x,
  // y, z) among the tangent space directions that people are
  // expecting.
  int storedcomponent;
  vector<int> indexvalues;
  bool invariant() const {
    bool inv = storedcomponent >= 0 &&
               storedcomponent < int(tensortype->storedcomponents.size()) &&
               int(indexvalues.size()) == tensortype->rank;
    for (int i = 0; i < int(indexvalues.size()); ++i)
      inv &= indexvalues[i] >= 0 && indexvalues[i] < tensortype->dimension;
    inv &= tensortype->storedcomponents[storedcomponent] == this;
    return inv;
  }
  TensorComponent(const string &name, TensorType *tensortype,
                  int storedcomponent, const std::vector<int> &indexvalues)
      : name(name), tensortype(tensortype), storedcomponent(storedcomponent),
        indexvalues(indexvalues) {
    tensortype->push_back(this);
    assert(invariant());
  }
};

TensorType Scalar3D("Scalar3D", 3, 0);
TensorComponent Scalar3D_scalar("scalar", &Scalar3D, 0, {});

TensorType Vector3D("Vector3D", 3, 1);
TensorComponent Vector3D_0("0", &Vector3D, 0, {0});
TensorComponent Vector3D_1("1", &Vector3D, 1, {1});
TensorComponent Vector3D_2("2", &Vector3D, 2, {2});

TensorType SymmetricTensor3D("SymmetricTensor3D", 3, 2);
TensorComponent SymmetricTensor3D_00("00", &SymmetricTensor3D, 0, {0, 0});
TensorComponent SymmetricTensor3D_01("01", &SymmetricTensor3D, 1, {0, 1});
TensorComponent SymmetricTensor3D_02("02", &SymmetricTensor3D, 2, {0, 2});
TensorComponent SymmetricTensor3D_11("11", &SymmetricTensor3D, 3, {1, 0});
TensorComponent SymmetricTensor3D_12("12", &SymmetricTensor3D, 4, {1, 1});
TensorComponent SymmetricTensor3D_22("22", &SymmetricTensor3D, 5, {2, 2});

// High-level continuum concepts

struct Manifold;
struct TangentSpace;
struct Field;
struct Discretization;
struct Basis;
struct DiscreteField;

set<Manifold *> manifolds;
set<TangentSpace *> tangentspaces;
set<Field *> Fields;

struct Manifold {
  string name;
  int dimension;
  set<Discretization *> discretizations;
  set<Field *> fields;
  bool invariant() const { return dimension >= 0; }
};

struct TangentSpace {
  string name;
  int dimension;
  set<Basis *> bases;
  set<Field *> fields;
  bool invariant() const { return dimension >= 0; }
};

struct Field {
  string name;
  Manifold *manifold;
  TangentSpace *tangentspace;
  TensorType *tensortype;
  set<DiscreteField> discretefields;
  bool invariant() const {
    return tangentspace->dimension == tensortype->dimension;
  }
};

// Manifold discretizations

struct DiscretizationBlock;

struct Discretization {
  string name;
  Manifold *manifold;
  set<DiscretizationBlock *> discretizationblocks;
  bool invariant() const { return true; }
};

struct DiscretizationBlock {
  // Discretization of a certain region, represented by contiguous data
  string name;
  Discretization *discretization;
  // bounding box? in terms of coordinates?
  // connectivity? neighbouring blocks?
  // overlaps?
  bool invariant() const { return true; }
};

// Tangent space bases

struct BasisVector;
struct CoordinateBasis;
struct CoordinateBasisElement;

struct Basis {
  string name;
  TangentSpace *tangentspace;
  vector<BasisVector *> basisvectors;
  set<CoordinateBasis *> coordinatebases;
  bool invariant() const {
    return int(basisvectors.size()) == tangentspace->dimension;
  }
};

struct BasisVector {
  string name;
  Basis *basis;
  // Since a BasisVector denotes essentially only an integer, we
  // should be able to replace it by an integer. Not sure this is
  // worthwhile. This essentially only gives names to directions; could use
  // vector<string> in TangentSpace for this instead.
  int direction;
  set<CoordinateBasisElement *> coordinatebasiselements;
  bool invariant() const {
    return direction >= 0 && direction < int(basis->basisvectors.size()) &&
           basis->basisvectors[direction] == this;
  }
};

// Discrete fields

struct DiscreteFieldBlock;
struct DiscreteFieldBlockData;

struct DiscreteField {
  string name;
  Field *field;
  Discretization *discretization;
  Basis *basis;
  set<DiscreteFieldBlock *> discretefieldblocks;
  bool invariant() const {
    return field->manifold == discretization->manifold &&
           field->tangentspace == basis->tangentspace;
  }
};

struct DiscreteFieldBlock {
  // Discrete field on a particular region (discretization block)
  string name;
  DiscreteField *discretefield;
  DiscretizationBlock *discretizationblock;
  set<DiscreteFieldBlockData *> discretefieldblockdata;
  bool invariant() const { return true; }
};

struct DiscreteFieldBlockData {
  // Tensor component for a discrete field on a particular region
  string name;
  DiscreteFieldBlock *discretefieldblock;
  TensorComponent *tensorcomponent;
  hid_t hdf5_dataset;
  bool invariant() const {
    return discretefieldblock->discretefield->field->tensortype ==
           tensorcomponent->tensortype;
  }
};

// Coordinates

struct CoordinateSystem;
struct CoordinateField;

set<CoordinateSystem *> coordinates;

struct CoordinateSystem {
  string name;
  Manifold *manifold;
  vector<CoordinateField *> coordinatefields;
  set<CoordinateBasis *> coordinatebases;
  bool invariant() const { return true; }
};

struct CoordinateField {
  CoordinateSystem *coordinatesystem;
  int direction;
  Field *field;
  bool invariant() const {
    return direction >= 0 &&
           direction < int(coordinatesystem->coordinatefields.size()) &&
           coordinatesystem->coordinatefields[direction] == this;
  }
};

struct CoordinateBasis {
  CoordinateSystem *coordinatesystem;
  Basis *basis;
  vector<CoordinateBasisElement *> coordinatebasiselements;
};

struct CoordinateBasisElement {
  CoordinateBasis *coordinatebasis;
  CoordinateField *coordinatefield;
  BasisVector *basisvector;
  bool invariant() const {
    return coordinatefield->direction == basisvector->direction;
  }
};
}

#endif // #SIMULATIONIO_HPP
