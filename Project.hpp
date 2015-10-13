#ifndef PROJECT_HPP
#define PROJECT_HPP

#include <H5Cpp.h>

#include "Common.hpp"

#include <cassert>
#include <map>
#include <iostream>
#include <string>

namespace SimulationIO {

using std::map;
using std::ostream;
using std::string;

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

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os, const Project &project) {
    return project.output(os);
  }
  virtual void write(const H5::CommonFG &loc,
                     const H5::H5Location &parent) const;
  void write(const H5::CommonFG &loc) const { write(loc, H5::H5File()); }

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
};
}

#define PROJECT_HPP_DONE
#endif // #ifndef PROJECT_HPP
#ifndef PROJECT_HPP_DONE
#error "Cyclic include depencency"
#endif
