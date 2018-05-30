#ifndef PROJECT_HPP
#define PROJECT_HPP

#include <asdf.hpp>

#include <H5Cpp.h>

#include "Common.hpp"
#include "Config.hpp"

#include "RegionCalculus.hpp"

#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace SimulationIO {

using std::make_shared;
using std::map;
using std::numeric_limits;
using std::ostream;
using std::shared_ptr;
using std::string;
using std::vector;

class Project;

shared_ptr<Project> createProject(const string &name);
shared_ptr<Project> readProject(const H5::H5Location &loc);
shared_ptr<Project> readProject(const ASDF::reader_state &rs,
                                const YAML::Node &node);

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
  virtual string type() const { return "Project"; }

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

  mutable vector<H5::CompType> linearizationtypes;
  mutable vector<H5::VarLenType> concatenationtypes;

  virtual bool invariant() const;

  Project(const Project &) = delete;
  Project(Project &&) = delete;
  Project &operator=(const Project &) = delete;
  Project &operator=(Project &&) = delete;

  friend shared_ptr<Project> createProject(const string &name);
  friend shared_ptr<Project> readProject(const H5::H5Location &loc);
  friend shared_ptr<Project> readProject(const ASDF::reader_state &rs,
                                         const YAML::Node &node);
  Project(hidden, const string &name) : Common(name) {
    SIMULATIONIO_CHECK_VERSION;
    createTypes();
  }
  Project(hidden) : Common(hidden()) { SIMULATIONIO_CHECK_VERSION; }

private:
  static shared_ptr<Project> create(const string &name) {
    auto project = make_shared<Project>(hidden(), name);
    project->createTypes();
    return project;
  }
  static shared_ptr<Project> create(const H5::H5Location &loc) {
    auto project = make_shared<Project>(hidden());
    project->read(loc);
    return project;
  }
  static shared_ptr<Project> create(const ASDF::reader_state &rs,
                                    const YAML::Node &node) {
    auto project = make_shared<Project>(hidden());
    project->read(rs, node);
    return project;
  }
  void read(const H5::H5Location &loc);
  void read(const ASDF::reader_state &rs, const YAML::Node &node);

public:
  virtual ~Project() {}

  void merge(const shared_ptr<Project> &project);

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
  virtual void write(const H5::H5Location &loc,
                     const H5::H5Location &parent) const;
  void write(const H5::H5Location &loc) const { write(loc, H5::H5File()); }
  virtual string yaml_alias() const;
  ASDF::writer &write(ASDF::writer &writer) const;
  friend ASDF::writer &operator<<(ASDF::writer &writer,
                                  const Project &project) {
    return project.write(writer);
  }

  shared_ptr<Parameter> createParameter(const string &name);
  shared_ptr<Parameter> getParameter(const string &name);
  shared_ptr<Parameter> copyParameter(const shared_ptr<Parameter> &parameter,
                                      bool copy_children = false);
  shared_ptr<Parameter> readParameter(const H5::H5Location &loc,
                                      const string &entry);
  shared_ptr<Parameter> readParameter(const ASDF::reader_state &rs,
                                      const YAML::Node &node);
  shared_ptr<Configuration> createConfiguration(const string &name);
  shared_ptr<Configuration> getConfiguration(const string &name);
  shared_ptr<Configuration>
  copyConfiguration(const shared_ptr<Configuration> &configuration,
                    bool copy_children = false);
  shared_ptr<Configuration> readConfiguration(const H5::H5Location &loc,
                                              const string &entry);
  shared_ptr<Configuration> readConfiguration(const ASDF::reader_state &rs,
                                              const YAML::Node &node);
  shared_ptr<TensorType> createTensorType(const string &name, int dimension,
                                          int rank);
  shared_ptr<TensorType> getTensorType(const string &name, int dimension,
                                       int rank);
  shared_ptr<TensorType>
  copyTensorType(const shared_ptr<TensorType> &tensortype,
                 bool copy_children = false);
  shared_ptr<TensorType> readTensorType(const H5::H5Location &loc,
                                        const string &entry);
  shared_ptr<TensorType> readTensorType(const ASDF::reader_state &rs,
                                        const YAML::Node &node);
  shared_ptr<Manifold>
  createManifold(const string &name,
                 const shared_ptr<Configuration> &configuration, int dimension);
  shared_ptr<Manifold>
  getManifold(const string &name,
              const shared_ptr<Configuration> &configuration, int dimension);
  shared_ptr<Manifold> copyManifold(const shared_ptr<Manifold> &manifold,
                                    bool copy_children = false);
  shared_ptr<Manifold> readManifold(const H5::H5Location &loc,
                                    const string &entry);
  shared_ptr<Manifold> readManifold(const ASDF::reader_state &rs,
                                    const YAML::Node &node);
  shared_ptr<TangentSpace>
  createTangentSpace(const string &name,
                     const shared_ptr<Configuration> &configuration,
                     int dimension);
  shared_ptr<TangentSpace>
  getTangentSpace(const string &name,
                  const shared_ptr<Configuration> &configuration,
                  int dimension);
  shared_ptr<TangentSpace>
  copyTangentSpace(const shared_ptr<TangentSpace> &tangentspace,
                   bool copy_children = false);
  shared_ptr<TangentSpace> readTangentSpace(const H5::H5Location &loc,
                                            const string &entry);
  shared_ptr<TangentSpace> readTangentSpace(const ASDF::reader_state &rs,
                                            const YAML::Node &node);
  shared_ptr<Field> createField(const string &name,
                                const shared_ptr<Configuration> &configuration,
                                const shared_ptr<Manifold> &manifold,
                                const shared_ptr<TangentSpace> &tangentspace,
                                const shared_ptr<TensorType> &tensortype);
  shared_ptr<Field> getField(const string &name,
                             const shared_ptr<Configuration> &configuration,
                             const shared_ptr<Manifold> &manifold,
                             const shared_ptr<TangentSpace> &tangentspace,
                             const shared_ptr<TensorType> &tensortype);
  shared_ptr<Field> copyField(const shared_ptr<Field> &field,
                              bool copy_children = false);
  shared_ptr<Field> readField(const H5::H5Location &loc, const string &entry);
  shared_ptr<Field> readField(const ASDF::reader_state &rs,
                              const YAML::Node &node);
  shared_ptr<CoordinateSystem>
  createCoordinateSystem(const string &name,
                         const shared_ptr<Configuration> &configuration,
                         const shared_ptr<Manifold> &manifold);
  shared_ptr<CoordinateSystem>
  getCoordinateSystem(const string &name,
                      const shared_ptr<Configuration> &configuration,
                      const shared_ptr<Manifold> &manifold);
  shared_ptr<CoordinateSystem>
  copyCoordinateSystem(const shared_ptr<CoordinateSystem> &coordinatesystem,
                       bool copy_children = false);
  shared_ptr<CoordinateSystem> readCoordinateSystem(const H5::H5Location &loc,
                                                    const string &entry);
  shared_ptr<CoordinateSystem>
  readCoordinateSystem(const ASDF::reader_state &rs, const YAML::Node &node);
};

} // namespace SimulationIO

#define PROJECT_HPP_DONE
#endif // #ifndef PROJECT_HPP
#ifndef PROJECT_HPP_DONE
#error "Cyclic include depencency"
#endif
