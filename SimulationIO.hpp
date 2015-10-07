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

// Tensor types

struct TensorType {
  int dimension;
  int rank;
  set<TensorComponent> storedcomponents;
  bool invariant() const {
    return dimension >= 0 && rank >= 0 &&
           storedcomponent.size() < pow(tensortype.dimension, tensortype.rank);
  }
};
set<TensorType> tensortypes;

struct TensorComponent {
  TensorType &tensortype;
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
               storedcomponent < tensortype.storedcomponents.size() &&
               indexvalues.size() == tensortype->rank;
    for (int iv : indexvalues)
      inv &= iv >= 0 && iv < tensortype->dimension;
    return inv;
  }
};

TensorType Scalar3D{"Scalar3D", 3, 0, {TensorComponent{Scalar3D, {}}}};

TensorType Vector3D{"Vector3D",
                    3,
                    1,
                    {TensorComponent{Vector3D, {0}},
                     TensorComponent{Vector3D, {1}},
                     TensorComponent{Vector3D, {2}}}};

TensorType SymmetricTensor3D{"SymmetricTensor3D",
                             3,
                             2,
                             {TensorComponent{SymmetricTensor3D, {0, 0}},
                              TensorComponent{SymmetricTensor3D, {0, 1}},
                              TensorComponent{SymmetricTensor3D, {0, 2}},
                              TensorComponent{SymmetricTensor3D, {1, 0}},
                              TensorComponent{SymmetricTensor3D, {1, 1}},
                              TensorComponent{SymmetricTensor3D, {2, 2}}}};

// High-level continuum concepts

struct Manifold {
  int dimension;
  set<Discretization> discretizations;
  set<Field *> fields;
  bool invariant() const { return dimension >= 0; }
};
set<Manifold> manifolds;

struct TangentSpace {
  int dimension;
  set<Basis> bases;
  set<Field *> fields;
  bool invariant() const { return dimension >= 0; }
};
set<TangentSpace> tangentspaces;

struct Field {
  Manifold *manifold;
  TangentSpace *tangentspace;
  TensorType *tensortype;
  set<DiscreteField> discretefields;
  bool invariant() const {
    return tangentspace->dimension == tensortype->dimension;
  }
};
set<Field> Fields;

// Manifold discretizations

struct Discretization {
  Manifold &manifold;
  set<DiscretizationBlock> discretizationblocks;
  bool invariant() const { return true; }
};

struct DiscretizationBlock {
  // Discretization of a certain region, represented by contiguous data
  Discretization &discretization;
  // bounding box? in terms of coordinates?
  // connectivity? neighbouring blocks?
  // overlaps?
  bool invariant() const { return true; }
};

// Tangent space bases

struct Basis {
  TangentSpace &tangentspace;
  vector<BasisVector> basisvectors;
  set<CoordinateBasis *> coordinatebases;
  bool invariant() const {
    return basisvectors.size() == tangentspace->dimension;
  }
};

struct BasisVector {
  Basis &basis;
  // Since a BasisVector denotes essentially only an integer, we
  // should be able to replace it by an integer. Not sure this is
  // worthwhile. This essentially only gives names to directions; could use
  // vector<string> in TangentSpace for this instead.
  int direction;
  set<CoordinateBasisElement *> coordinatebasiselements;
  bool invariant() const {
    return direction >= 0 && direction < basis->basisvectors.size() &&
           basis->basisvectors[direction] == this;
  }
};

// Discrete fields

struct DiscreteField {
  Field &field;
  Discretization *discretization;
  Basis *basis;
  set<DiscreteFieldBlock> discretefieldblocks;
  bool invariant() const {
    return field->manifold == discretization->manifold &&
           field->tangentspace == basis->tangentspace;
  }
};

struct DiscreteFieldBlock {
  // Discrete field on a particular region (discretization block)
  DiscreteField &discretefield;
  DiscretizationBlock *discretizationblock;
  set<DiscreteFieldBlockData> discretefieldblockdata;
  bool invariant() const { return true; }
};

struct DiscreteFieldBlockData {
  // Tensor component for a discrete field on a particular region
  DiscreteFieldBlock &discretefieldblock;
  TensorComponent *tensorcomponent;
  hid_t hdf5_dataset;
  bool invariant() const {
    return discretefieldblock.discretefield->field->tensortype ==
           tensorcomponent->tensortype;
  }
};

// Coordinates

struct CoordinateSystem {
  Manifold *manifold;
  vector<CoordinateField> coordinatefields;
  set<CoordinateBasis *> coordinatebases;
  bool invariant() const { return true; }
};
set<Coordinate> coordinates;

struct CoordinateField {
  CoordinateSystem &coordinatesystem;
  int direction;
  Field *field;
  bool invariant() const {
    return direction >= 0 &&
           direction < coordinatesystem->coordinatefields.size() &&
           coordinatesystem->coordinatefields[direction] == this;
  }
};

struct CoordinateBasis {
  CoordinateSystem *coordinatesystem;
  Basis *basis;
  vector<CoordinateBasisElement> coordinatebasiselements;
};

struct CoordinateBasisElement {
  CoordinateBasis &coordinatebasis;
  CoordinateField *coordinatefield;
  BasisVector *basisvector;
  bool invariant() const {
    return coordinatefield->direction == basisvector->direction;
  }
};
