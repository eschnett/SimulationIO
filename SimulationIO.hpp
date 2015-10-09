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

#include <H5Cpp.h>

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
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

// Indented output
const int indentsize = 2;
const char indentchar = ' ';
inline string indent(int level) {
  return string(level * indentsize, indentchar);
}

// HDF5 helpers

inline H5::DataType H5type(const char &) {
  return H5::IntType(H5::PredType::NATIVE_CHAR);
}
inline H5::DataType h5type(const signed char &) {
  return H5::IntType(H5::PredType::NATIVE_SCHAR);
}
inline H5::DataType h5type(const unsigned char &) {
  return H5::IntType(H5::PredType::NATIVE_UCHAR);
}
inline H5::DataType h5type(const short &) {
  return H5::IntType(H5::PredType::NATIVE_SHORT);
}
inline H5::DataType h5type(const unsigned short &) {
  return H5::IntType(H5::PredType::NATIVE_USHORT);
}
inline H5::DataType h5type(const int &) {
  return H5::IntType(H5::PredType::NATIVE_INT);
}
inline H5::DataType h5type(const unsigned int &) {
  return H5::IntType(H5::PredType::NATIVE_UINT);
}
inline H5::DataType h5type(const long &) {
  return H5::IntType(H5::PredType::NATIVE_LONG);
}
inline H5::DataType h5type(const unsigned long &) {
  return H5::IntType(H5::PredType::NATIVE_ULONG);
}
inline H5::DataType h5type(const long long &) {
  return H5::IntType(H5::PredType::NATIVE_LLONG);
}
inline H5::DataType h5type(const unsigned long long &) {
  return H5::IntType(H5::PredType::NATIVE_ULLONG);
}
inline H5::DataType h5type(const float &) {
  return H5::FloatType(H5::PredType::NATIVE_FLOAT);
}
inline H5::DataType h5type(const double &) {
  return H5::FloatType(H5::PredType::NATIVE_DOUBLE);
}
inline H5::DataType h5type(const long double &) {
  return H5::FloatType(H5::PredType::NATIVE_LDOUBLE);
}

// H5Literate
namespace detail {
template <typename Op> struct H5L_iterator {
  Op &&op;
  static herr_t call(hid_t g_id, const char *name, const H5L_info_t *info,
                     void *data) {
    return static_cast<H5L_iterator *>(data)->op(H5::Group(g_id), name, info);
  }
  herr_t operator()(const H5::Group &group, H5_index_t index_type,
                    H5_iter_order_t order, hsize_t *idx) {
    return H5Literate(group.getId(), index_type, order, idx, call, this);
  }
};
}

template <typename Op>
herr_t H5_iterate(const H5::Group &group, H5_index_t index_type,
                  H5_iter_order_t order, hsize_t *idx, Op &&op) {
  return detail::H5L_iterator<Op>{std::forward<Op>(op)}(group.getId(),
                                                        index_type, order, idx);
}

// Common to all file elements

struct Common {
  string name;
  Common(const string &name) : name(name) {}
  virtual ~Common() {}
  virtual bool invariant() const { return !name.empty(); }
  virtual ostream &output(ostream &os, int level = 0) const = 0;
  virtual void write(H5::CommonFG &loc) const = 0;
};

// Projects

struct TensorType;
struct Manifold;
struct TangentSpace;
struct Field;
struct CoordinateSystem;

struct Project : Common {
  map<string, TensorType *> tensortypes;
  map<string, Manifold *> manifolds;
  map<string, TangentSpace *> tangentspaces;
  map<string, Field *> fields;
  map<string, CoordinateSystem *> coordinatesystems;
  virtual bool invariant() const override { return Common::invariant(); }
  Project(const Project &) = delete;
  Project(Project &&) = delete;
  Project &operator=(const Project &) = delete;
  Project &operator=(Project &&) = delete;
  Project(const string &name) : Common(name) {}
  virtual ~Project() override { assert(0); }
  void create_standard_tensortypes();
  void insert(const string &name, TensorType *tensortype) {
    assert(!tensortypes.count(name));
    // TODO: use emplace
    tensortypes[name] = tensortype;
  }
  void insert(const string &name, Manifold *manifold) {
    assert(!manifolds.count(name));
    manifolds[name] = manifold;
  }
  void insert(const string &name, TangentSpace *tangentspace) {
    assert(!tangentspaces.count(name));
    tangentspaces[name] = tangentspace;
  }
  void insert(const string &name, Field *field) {
    assert(!fields.count(name));
    fields[name] = field;
  }
  void insert(const string &name, CoordinateSystem *coordinatesystem) {
    assert(!coordinatesystems.count(name));
    coordinatesystems[name] = coordinatesystem;
  }
  virtual ostream &output(ostream &os, int level = 0) const override;
  friend ostream &operator<<(ostream &os, const Project &project) {
    return project.output(os);
  }
  virtual void write(H5::CommonFG &loc) const override;
  Project(const string &name, const H5::CommonFG &loc);
};

// Tensor types

struct TensorComponent;

struct TensorType : Common {
  Project *project;
  int dimension;
  int rank;
  map<string, TensorComponent *> tensorcomponents; // owned
  virtual bool invariant() const override {
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
  TensorType(const string &name, Project *project, int dimension, int rank)
      : Common(name), project(project), dimension(dimension), rank(rank) {
    project->insert(name, this);
    assert(invariant());
  }
  virtual ~TensorType() override { assert(0); }
  void insert(const string &name, TensorComponent *tensorcomponent) {
    assert(!tensorcomponents.count(name));
    tensorcomponents[name] = tensorcomponent;
  }
  virtual ostream &output(ostream &os, int level = 0) const override;
  friend ostream &operator<<(ostream &os, const TensorType &tensortype) {
    return tensortype.output(os);
  }
  virtual void write(H5::CommonFG &loc) const override;
  TensorType(const string &name, Project *project, const H5::CommonFG &loc);
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
  virtual bool invariant() const override {
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
  TensorComponent(const string &name, TensorType *tensortype,
                  const std::vector<int> &indexvalues)
      : Common(name), tensortype(tensortype), indexvalues(indexvalues) {
    tensortype->insert(name, this);
    assert(invariant());
  }
  virtual ~TensorComponent() override { assert(0); }
  virtual ostream &output(ostream &os, int level = 0) const override;
  friend ostream &operator<<(ostream &os,
                             const TensorComponent &tensorcomponent) {
    return tensorcomponent.output(os);
  }
  virtual void write(H5::CommonFG &loc) const override;
  TensorComponent(const string &name, TensorType *tensortype,
                  const H5::CommonFG &loc);
};

// High-level continuum concepts

struct Discretization;
struct Basis;
struct DiscreteField;

struct Manifold : Common {
  Project *project;
  int dimension;
  map<string, Discretization *> discretizations;
  map<string, Field *> fields;
  virtual bool invariant() const override {
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
  Manifold(const string &name, Project *project, int dimension)
      : Common(name), project(project), dimension(dimension) {
    project->insert(name, this);
    assert(invariant());
  }
  virtual ~Manifold() override { assert(0); }
  void insert(const string &name, Field *field) {
    assert(!fields.count(name));
    fields[name] = field;
  }
  virtual ostream &output(ostream &os, int level = 0) const override;
  friend ostream &operator<<(ostream &os, const Manifold &manifold) {
    return manifold.output(os);
  }
  virtual void write(H5::CommonFG &loc) const override;
  Manifold(const string &name, H5::CommonFG &loc);
};

struct TangentSpace : Common {
  Project *project;
  int dimension;
  map<string, Basis *> bases;
  map<string, Field *> fields;
  virtual bool invariant() const override {
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
  TangentSpace(const string &name, Project *project, int dimension)
      : Common(name), project(project), dimension(dimension) {
    project->insert(name, this);
    assert(invariant());
  }
  virtual ~TangentSpace() override { assert(0); }
  void insert(const string &name, Field *field) {
    assert(!fields.count(name));
    fields[name] = field;
  }
  virtual ostream &output(ostream &os, int level = 0) const override;
  friend ostream &operator<<(ostream &os, const TangentSpace &tangentspace) {
    return tangentspace.output(os);
  }
  virtual void write(H5::CommonFG &loc) const override;
  TangentSpace(const string &name, H5::CommonFG &loc);
};

struct Field : Common {
  Project *project;
  Manifold *manifold;
  TangentSpace *tangentspace;
  TensorType *tensortype;
  map<string, DiscreteField *> discretefields;
  virtual bool invariant() const override {
    bool inv = Common::invariant() && bool(project) &&
               project->fields.count(name) &&
               project->fields.at(name) == this && bool(manifold) &&
               bool(tangentspace) && bool(tensortype) &&
               tangentspace->dimension == tensortype->dimension &&
               manifold->fields.at(name) == this &&
               tangentspace->fields.at(name) == this;
    for (const auto &df : discretefields)
      inv &= !df.first.empty() && bool(df.second);
    return inv;
  }
  Field() = delete;
  Field(const Field &) = delete;
  Field(Field &&) = delete;
  Field &operator=(const Field &) = delete;
  Field &operator=(Field &&) = delete;
  Field(const string &name, Project *project, Manifold *manifold,
        TangentSpace *tangentspace, TensorType *tensortype)
      : Common(name), project(project), manifold(manifold),
        tangentspace(tangentspace), tensortype(tensortype) {
    project->insert(name, this);
    manifold->insert(name, this);
    tangentspace->insert(name, this);
    // tensortypes->insert(this);
    assert(invariant());
  }
  virtual ~Field() override { assert(0); }
  virtual ostream &output(ostream &os, int level = 0) const override;
  friend ostream &operator<<(ostream &os, const Field &field) {
    return field.output(os);
  }
  virtual void write(H5::CommonFG &loc) const override;
  Field(const string &name, H5::CommonFG &loc);
};

// Manifold discretizations

struct DiscretizationBlock;

struct Discretization {
  string name;
  Manifold *manifold;
  map<string, DiscretizationBlock *> discretizationblocks;
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
  map<string, DiscreteFieldBlock *> discretefieldblocks;
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
  map<string, DiscreteFieldBlockData *> discretefieldblockdata;
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
  vector<CoordinateField *> coordinatefields;
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
