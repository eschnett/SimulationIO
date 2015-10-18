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

struct Parameter;
struct Configuration;
struct TensorType;
struct Manifold;
struct TangentSpace;
struct Field;
// struct CoordinateSystem;
// struct CoordinateBasis;

struct Project : Common {
  map<string, Parameter *> parameters;         // children
  map<string, Configuration *> configurations; // children
  map<string, TensorType *> tensortypes;       // children
  map<string, Manifold *> manifolds;           // children
  map<string, TangentSpace *> tangentspaces;   // children
  map<string, Field *> fields;                 // children
  // map<string, CoordinateSystem *> coordinatesystems; // children
  // TODO: coordinatebasis

  mutable H5::EnumType enumtype;

  virtual bool invariant() const { return Common::invariant(); }

  Project(const Project &) = delete;
  Project(Project &&) = delete;
  Project &operator=(const Project &) = delete;
  Project &operator=(Project &&) = delete;

private:
  friend Project *createProject(const string &name);
  friend Project *createProject(const H5::CommonFG &loc, const string &entry);
  Project(const string &name) : Common(name) { createTypes(); }
  Project(const H5::CommonFG &loc, const string &entry);

public:
  virtual ~Project() { assert(0); }

  void createStandardTensortypes();

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os, const Project &project) {
    return project.output(os);
  }

private:
  static void insertEnumField(const H5::EnumType &type, const string &name,
                              int value);
  void createTypes() const;

public:
  virtual void write(const H5::CommonFG &loc,
                     const H5::H5Location &parent) const;
  void write(const H5::CommonFG &loc) const { write(loc, H5::H5File()); }

  Parameter *createParameter(const string &name);
  Parameter *createParameter(const H5::CommonFG &loc, const string &entry);
  Configuration *createConfiguration(const string &name);
  Configuration *createConfiguration(const H5::CommonFG &loc,
                                     const string &entry);
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
  // CoordinateSystem *createCoordinateSystem(const string &name,
  //                                          Manifold *manifold);
  // CoordinateSystem *createCoordinateSystem(const H5::CommonFG &loc,
  //                                          const string &entry);
};
}

#define PROJECT_HPP_DONE
#endif // #ifndef PROJECT_HPP
#ifndef PROJECT_HPP_DONE
#error "Cyclic include depencency"
#endif
