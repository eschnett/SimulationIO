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

#include "H5Helpers.hpp"

#include <H5Cpp.h>

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace SimulationIO {

using std::map;
using std::ostream;
using std::string;
using std::vector;

// Integer exponentiation
inline int ipow(int base, int exp) {
  assert(base >= 0 && exp >= 0);
  int res = 1;
  while (exp--)
    res *= base;
  return res;
}

// Insert an element into a map; ensure that the key does not yet exist
template <typename Key, typename Value, typename Key1, typename Value1>
typename map<Key, Value>::iterator checked_insert(map<Key, Value> &m,
                                                  Key1 &&key, Value1 &&value) {
  typename map<Key, Value>::iterator iter;
  bool did_insert;
  std::tie(iter, did_insert) =
      m.insert(make_pair(std::forward<Key1>(key), std::forward<Value1>(value)));
  assert(did_insert);
  return iter;
}

// Indented output
const int indentsize = 2;
const char indentchar = ' ';
inline string indent(int level) {
  return string(level * indentsize, indentchar);
}

// Common to all file elements

struct Common {
  string name;
  Common(const string &name) : name(name) {}
  virtual ~Common() {}
  virtual bool invariant() const { return !name.empty(); }
  virtual ostream &output(ostream &os, int level = 0) const = 0;
  virtual void write(const H5::CommonFG &loc,
                     const H5::H5Location &parent) const = 0;
};

// Projects

struct Project;

Project *createProject(const string &name);
Project *createProject(const H5::CommonFG &loc, const string &entry);

struct TensorType;
struct Manifold;
struct TangentSpace;
struct Field;
struct CoordinateSystem;
struct CoordinateBasis;

struct Project : Common {
  map<string, TensorType *> tensortypes;             // owned
  map<string, Manifold *> manifolds;                 // owned
  map<string, TangentSpace *> tangentspaces;         // owned
  map<string, Field *> fields;                       // owned
  map<string, CoordinateSystem *> coordinatesystems; // owned
  // TODO: coordinatebasis
  virtual bool invariant() const { return Common::invariant(); }
  Project(const Project &) = delete;
  Project(Project &&) = delete;
  Project &operator=(const Project &) = delete;
  Project &operator=(Project &&) = delete;

private:
  friend Project *createProject(const string &name);
  friend Project *createProject(const H5::CommonFG &loc, const string &entry);
  Project(const string &name) : Common(name) {}
  Project(const H5::CommonFG &loc, const string &entry);

public:
  virtual ~Project() { assert(0); }
  void createStandardTensortypes();
  TensorType *createTensorType(const string &name, int dimension, int rank);
  TensorType *createTensorType(const H5::CommonFG &loc, const string &entry);
  Manifold *createManifold(const string &name, int dimension);
  Manifold *createManifold(const H5::CommonFG &loc, const string &entry);
  TangentSpace *createTangentSpace(const string &name, int dimension);
  TangentSpace *createTangentSpace(const H5::CommonFG &loc,
                                   const string &entry);
  Field *createField(const string &name, Manifold *manifold,
                     TangentSpace *tangentspace, TensorType *tensortype);
  Field *createField(const H5::CommonFG &loc, const string &entry);
  CoordinateSystem *createCoordinateSystem(const string &name,
                                           Manifold *manifold);
  CoordinateSystem *createCoordinateSystem(const H5::CommonFG &loc,
                                           const string &entry);
  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os, const Project &project) {
    return project.output(os);
  }
  virtual void write(const H5::CommonFG &loc,
                     const H5::H5Location &parent) const;
};

// Tensor types

struct TensorComponent;

struct TensorType : Common {
  Project *project;
  int dimension;
  int rank;
  map<string, TensorComponent *> tensorcomponents; // owned
  virtual bool invariant() const {
    bool inv = Common::invariant() && bool(project) &&
               project->tensortypes.count(name) &&
               project->tensortypes.at(name) == this && dimension >= 0 &&
               rank >= 0 &&
               int(tensorcomponents.size()) <= ipow(dimension, rank);
    for (const auto &tc : tensorcomponents)
      inv &= !tc.first.empty() && bool(tc.second);
    return inv;
  }
  TensorType() = delete;
  TensorType(const TensorType &) = delete;
  TensorType(TensorType &&) = delete;
  TensorType &operator=(const TensorType &) = delete;
  TensorType &operator=(TensorType &&) = delete;

private:
  friend class Project;
  TensorType(const string &name, Project *project, int dimension, int rank)
      : Common(name), project(project), dimension(dimension), rank(rank) {}
  TensorType(const H5::CommonFG &loc, const string &entry, Project *project);

public:
  virtual ~TensorType() { assert(0); }
  TensorComponent *createTensorComponent(const string &name,
                                         const std::vector<int> &indexvalues);
  TensorComponent *createTensorComponent(const H5::CommonFG &loc,
                                         const string &entry);
  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os, const TensorType &tensortype) {
    return tensortype.output(os);
  }
  virtual void write(const H5::CommonFG &loc,
                     const H5::H5Location &parent) const;
};

struct TensorComponent : Common {
  TensorType *tensortype;
  // We use objects to denote most Commons, but we make an exception
  // for tensor component indices and tangent space basis vectors,
  // which we number consecutively starting from zero. This simplifies
  // the representation, and it introduces a canonical order (e.g. x,
  // y, z) among the tangent space directions that people are
  // expecting.
  vector<int> indexvalues;
  virtual bool invariant() const {
    bool inv = Common::invariant() && bool(tensortype) &&
               tensortype->tensorcomponents[name] == this &&
               int(indexvalues.size()) == tensortype->rank;
    for (int i = 0; i < int(indexvalues.size()); ++i)
      inv &= indexvalues[i] >= 0 && indexvalues[i] < tensortype->dimension;
    // Ensure all tensor components are distinct
    for (const auto &tc : tensortype->tensorcomponents) {
      const auto &other = tc.second;
      if (other == this)
        continue;
      bool samesize = other->indexvalues.size() == indexvalues.size();
      inv &= samesize;
      if (samesize) {
        bool isequal = true;
        for (int i = 0; i < int(indexvalues.size()); ++i)
          isequal &= other->indexvalues[i] == indexvalues[i];
        inv &= !isequal;
      }
    }
    return inv;
  }
  TensorComponent() = delete;
  TensorComponent(const TensorComponent &) = delete;
  TensorComponent(TensorComponent &&) = delete;
  TensorComponent &operator=(const TensorComponent &) = delete;
  TensorComponent &operator=(TensorComponent &&) = delete;

private:
  friend class TensorType;
  TensorComponent(const string &name, TensorType *tensortype,
                  const std::vector<int> &indexvalues)
      : Common(name), tensortype(tensortype), indexvalues(indexvalues) {}
  TensorComponent(const H5::CommonFG &loc, const string &entry,
                  TensorType *tensortype);

public:
  virtual ~TensorComponent() { assert(0); }
  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os,
                             const TensorComponent &tensorcomponent) {
    return tensorcomponent.output(os);
  }
  virtual void write(const H5::CommonFG &loc,
                     const H5::H5Location &parent) const;
};

// High-level continuum concepts

struct Discretization;
struct Basis;
struct DiscreteField;

struct Manifold : Common {
  Project *project;
  int dimension;
  map<string, Discretization *> discretizations; // owned
  map<string, Field *> fields;
  virtual bool invariant() const {
    bool inv = Common::invariant() && bool(project) &&
               project->manifolds.count(name) &&
               project->manifolds.at(name) == this && dimension >= 0;
    for (const auto &d : discretizations)
      inv &= !d.first.empty() && bool(d.second);
    for (const auto &f : fields)
      inv &= !f.first.empty() && bool(f.second);
    return inv;
  }
  Manifold() = delete;
  Manifold(const Manifold &) = delete;
  Manifold(Manifold &&) = delete;
  Manifold &operator=(const Manifold &) = delete;
  Manifold &operator=(Manifold &&) = delete;

private:
  friend class Project;
  Manifold(const string &name, Project *project, int dimension)
      : Common(name), project(project), dimension(dimension) {}
  Manifold(const H5::CommonFG &loc, const string &entry, Project *project);

public:
  virtual ~Manifold() { assert(0); }
  void insert(const string &name, Field *field) {
    assert(!fields.count(name));
    fields[name] = field;
  }
  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os, const Manifold &manifold) {
    return manifold.output(os);
  }
  virtual void write(const H5::CommonFG &loc,
                     const H5::H5Location &parent) const;
};

struct TangentSpace : Common {
  Project *project;
  int dimension;
  map<string, Basis *> bases; // owned
  map<string, Field *> fields;
  virtual bool invariant() const {
    bool inv = Common::invariant() && bool(project) &&
               project->tangentspaces.count(name) &&
               project->tangentspaces.at(name) == this && dimension >= 0;
    for (const auto &b : bases)
      inv &= !b.first.empty() && bool(b.second);
    for (const auto &f : fields)
      inv &= !f.first.empty() && bool(f.second);
    return inv;
  }
  TangentSpace() = delete;
  TangentSpace(const TangentSpace &) = delete;
  TangentSpace(TangentSpace &&) = delete;
  TangentSpace &operator=(const TangentSpace &) = delete;
  TangentSpace &operator=(TangentSpace &&) = delete;

private:
  friend class Project;
  TangentSpace(const string &name, Project *project, int dimension)
      : Common(name), project(project), dimension(dimension) {}
  TangentSpace(const H5::CommonFG &loc, const string &entry, Project *project);

public:
  virtual ~TangentSpace() { assert(0); }
  void insert(const string &name, Field *field) {
    assert(!fields.count(name));
    fields[name] = field;
  }
  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os, const TangentSpace &tangentspace) {
    return tangentspace.output(os);
  }
  virtual void write(const H5::CommonFG &loc,
                     const H5::H5Location &parent) const;
};

struct Field : Common {
  Project *project;
  Manifold *manifold;
  TangentSpace *tangentspace;
  TensorType *tensortype;
  map<string, DiscreteField *> discretefields; // owned
  virtual bool invariant() const {
    bool inv =
        Common::invariant() && bool(project) && project->fields.count(name) &&
        project->fields.at(name) == this && bool(manifold) &&
        manifold->fields.count(name) && manifold->fields.at(name) == this &&
        bool(tangentspace) && tangentspace->fields.count(name) &&
        tangentspace->fields.at(name) == this && bool(tensortype) &&
        tangentspace->dimension == tensortype->dimension;
    for (const auto &df : discretefields)
      inv &= !df.first.empty() && bool(df.second);
    return inv;
  }
  Field() = delete;
  Field(const Field &) = delete;
  Field(Field &&) = delete;
  Field &operator=(const Field &) = delete;
  Field &operator=(Field &&) = delete;

private:
  friend class Project;
  Field(const string &name, Project *project, Manifold *manifold,
        TangentSpace *tangentspace, TensorType *tensortype)
      : Common(name), project(project), manifold(manifold),
        tangentspace(tangentspace), tensortype(tensortype) {
    manifold->insert(name, this);
    tangentspace->insert(name, this);
    // tensortypes->insert(this);
  }
  Field(const H5::CommonFG &loc, const string &entry, Project *project);

public:
  virtual ~Field() { assert(0); }
  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os, const Field &field) {
    return field.output(os);
  }
  virtual void write(const H5::CommonFG &loc,
                     const H5::H5Location &parent) const;
};

// Manifold discretizations

struct DiscretizationBlock;

struct Discretization {
  string name;
  Manifold *manifold;
  map<string, DiscretizationBlock *> discretizationblocks; // owned
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
  vector<BasisVector *> basisvectors; // owned
  map<string, CoordinateBasis *> coordinatebases;
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
  map<string, CoordinateBasisElement *> coordinatebasiselements;
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
  map<string, DiscreteFieldBlock *> discretefieldblocks; // owned
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
  map<string, DiscreteFieldBlockData *> discretefieldblockdata; // owned
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

struct CoordinateField;

struct CoordinateSystem {
  string name;
  Manifold *manifold;
  vector<CoordinateField *> coordinatefields; // owned
  map<string, CoordinateBasis *> coordinatebases;
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
  vector<CoordinateBasisElement *> coordinatebasiselements; // owned
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
