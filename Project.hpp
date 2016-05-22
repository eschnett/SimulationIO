#ifndef PROJECT_HPP
#define PROJECT_HPP

#include <H5Cpp.h>

#include "Common.hpp"

#include "RegionCalculus.hpp"

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace SimulationIO {

using std::make_shared;
using std::map;
using std::ostream;
using std::shared_ptr;
using std::string;
using std::vector;

class Project;

shared_ptr<Project> createProject(const string &name);
shared_ptr<Project> readProject(const H5::CommonFG &loc);

class Parameter;
class Configuration;
class CoordinateSystem;
class TensorType;
class Manifold;
class TangentSpace;
class Field;
// class CoordinateSystem;
// class CoordinateBasis;

class Project : public Common, public std::enable_shared_from_this<Project> {
  map<string, shared_ptr<Parameter>> m_parameters;               // children
  map<string, shared_ptr<Configuration>> m_configurations;       // children
  map<string, shared_ptr<TensorType>> m_tensortypes;             // children
  map<string, shared_ptr<Manifold>> m_manifolds;                 // children
  map<string, shared_ptr<TangentSpace>> m_tangentspaces;         // children
  map<string, shared_ptr<Field>> m_fields;                       // children
  map<string, shared_ptr<CoordinateSystem>> m_coordinatesystems; // children
  // TODO: coordinatebasis
public:
  const map<string, shared_ptr<Parameter>> &parameters() const {
    return m_parameters;
  }
  const map<string, shared_ptr<Configuration>> &configurations() const {
    return m_configurations;
  }
  const map<string, shared_ptr<TensorType>> &tensortypes() const {
    return m_tensortypes;
  }
  const map<string, shared_ptr<Manifold>> &manifolds() const {
    return m_manifolds;
  }
  const map<string, shared_ptr<TangentSpace>> &tangentspaces() const {
    return m_tangentspaces;
  }
  const map<string, shared_ptr<Field>> &fields() const { return m_fields; }
  const map<string, shared_ptr<CoordinateSystem>> &coordinatesystems() const {
    return m_coordinatesystems;
  }

  mutable H5::EnumType enumtype;
  mutable H5::CompType rangetype;

  mutable vector<H5::ArrayType> pointtypes;
  mutable vector<H5::CompType> boxtypes;
  mutable vector<H5::VarLenType> regiontypes;

  virtual bool invariant() const { return Common::invariant(); }

  Project(const Project &) = delete;
  Project(Project &&) = delete;
  Project &operator=(const Project &) = delete;
  Project &operator=(Project &&) = delete;

  friend shared_ptr<Project> createProject(const string &name);
  friend shared_ptr<Project> readProject(const H5::CommonFG &loc);
  Project(hidden, const string &name) : Common(name) { createTypes(); }
  Project(hidden) : Common(hidden()) {}

private:
  static shared_ptr<Project> create(const string &name) {
    auto project = make_shared<Project>(hidden(), name);
    project->createTypes();
    return project;
  }
  static shared_ptr<Project> create(const H5::CommonFG &loc) {
    auto project = make_shared<Project>(hidden());
    project->read(loc);
    return project;
  }
  void read(const H5::CommonFG &loc);

public:
  virtual ~Project() {}

  void createStandardTensorTypes();

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

  shared_ptr<Parameter> createParameter(const string &name);
  shared_ptr<Parameter> readParameter(const H5::CommonFG &loc,
                                      const string &entry);
  shared_ptr<Configuration> createConfiguration(const string &name);
  shared_ptr<Configuration> readConfiguration(const H5::CommonFG &loc,
                                              const string &entry);
  shared_ptr<TensorType> createTensorType(const string &name, int dimension,
                                          int rank);
  shared_ptr<TensorType> readTensorType(const H5::CommonFG &loc,
                                        const string &entry);
  shared_ptr<Manifold>
  createManifold(const string &name,
                 const shared_ptr<Configuration> &configuration, int dimension);
  shared_ptr<Manifold> readManifold(const H5::CommonFG &loc,
                                    const string &entry);
  shared_ptr<TangentSpace>
  createTangentSpace(const string &name,
                     const shared_ptr<Configuration> &configuration,
                     int dimension);
  shared_ptr<TangentSpace> readTangentSpace(const H5::CommonFG &loc,
                                            const string &entry);
  shared_ptr<Field> createField(const string &name,
                                const shared_ptr<Configuration> &configuration,
                                const shared_ptr<Manifold> &manifold,
                                const shared_ptr<TangentSpace> &tangentspace,
                                const shared_ptr<TensorType> &tensortype);
  shared_ptr<Field> readField(const H5::CommonFG &loc, const string &entry);
  shared_ptr<CoordinateSystem>
  createCoordinateSystem(const string &name,
                         const shared_ptr<Configuration> &configuration,
                         const shared_ptr<Manifold> &manifold);
  shared_ptr<CoordinateSystem> readCoordinateSystem(const H5::CommonFG &loc,
                                                    const string &entry);
};
}

#define PROJECT_HPP_DONE
#endif // #ifndef PROJECT_HPP
#ifndef PROJECT_HPP_DONE
#error "Cyclic include depencency"
#endif
