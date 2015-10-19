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

#include "Basis.hpp"
#include "BasisVector.hpp"
#include "Common.hpp"
#include "Configuration.hpp"
#include "DiscreteField.hpp"
#include "DiscreteFieldBlock.hpp"
#include "DiscreteFieldBlockData.hpp"
#include "Discretization.hpp"
#include "DiscretizationBlock.hpp"
#include "Field.hpp"
#include "Manifold.hpp"
#include "Parameter.hpp"
#include "ParameterValue.hpp"
#include "Project.hpp"
#include "TangentSpace.hpp"
#include "TensorComponent.hpp"
#include "TensorType.hpp"

#include <cassert>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace SimulationIO {

using std::map;
using std::ostream;
using std::shared_ptr;
using std::string;
using std::vector;

// Coordinates

struct CoordinateBasis;
struct CoordinateField;

struct CoordinateSystem {
  string name;
  shared_ptr<Manifold> manifold;
  vector<shared_ptr<CoordinateField>> coordinatefields; // owned
  map<string, shared_ptr<CoordinateBasis>> coordinatebases;
  bool invariant() const { return true; }
};

struct CoordinateField {
  shared_ptr<CoordinateSystem> coordinatesystem;
  int direction;
  shared_ptr<Field> field;
  bool invariant() const {
    return direction >= 0 &&
           direction < int(coordinatesystem->coordinatefields.size()) &&
           coordinatesystem->coordinatefields.at(direction).get() == this;
  }
};

struct CoordinateBasisElement;

struct CoordinateBasis {
  shared_ptr<CoordinateSystem> coordinatesystem;
  shared_ptr<Basis> basis;
  vector<shared_ptr<CoordinateBasisElement>> coordinatebasiselements; // owned
};

struct CoordinateBasisElement {
  shared_ptr<CoordinateBasis> coordinatebasis;
  shared_ptr<CoordinateField> coordinatefield;
  shared_ptr<BasisVector> basisvector;
  bool invariant() const {
    return coordinatefield->direction == basisvector->direction;
  }
};
}

#define SIMULATIONIO_HPP_DONE
#endif // #ifndef SIMULATIONIO_HPP
#ifndef SIMULATIONIO_HPP_DONE
#error "Cyclic include depencency"
#endif
