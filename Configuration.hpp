#ifndef CONFIGURATION_HPP
#define CONFIGURATION_HPP

#include "Common.hpp"
#include "Helpers.hpp"
#include "Project.hpp"

#include <H5Cpp.h>

#include <iostream>
#include <map>
#include <memory>
#include <string>

namespace SimulationIO {

using std::make_shared;
using std::map;
using std::ostream;
using std::shared_ptr;
using std::string;
using std::weak_ptr;

class Basis;
class CoordinateSystem;
class DiscreteField;
class Discretization;
class Field;
class Manifold;
class ParameterValue;
class TangentSpace;

class Configuration : public Common,
                      public std::enable_shared_from_this<Configuration> {
  weak_ptr<Project> m_project;                                 // parent
  map<string, shared_ptr<ParameterValue>> m_parametervalues;   // links
  map<string, weak_ptr<Basis>> m_bases;                        // backlinks
  map<string, weak_ptr<CoordinateSystem>> m_coordinatesystems; // backlinks
  map<string, weak_ptr<DiscreteField>> m_discretefields;       // backlinks
  map<string, weak_ptr<Discretization>> m_discretizations;     // backlinks
  map<string, weak_ptr<Field>> m_fields;                       // backlinks
  map<string, weak_ptr<Manifold>> m_manifolds;                 // backlinks
  map<string, weak_ptr<TangentSpace>> m_tangentspaces;         // backlinks
public:
  shared_ptr<Project> project() const { return m_project.lock(); }
  const map<string, shared_ptr<ParameterValue>> &parametervalues() const {
    return m_parametervalues;
  }
  const map<string, weak_ptr<Basis>> &bases() const { return m_bases; }
  const map<string, weak_ptr<CoordinateSystem>> &coordinatesystems() const {
    return m_coordinatesystems;
  }
  const map<string, weak_ptr<DiscreteField>> &discretefields() const {
    return m_discretefields;
  }
  const map<string, weak_ptr<Discretization>> &discretizations() const {
    return m_discretizations;
  }
  const map<string, weak_ptr<Field>> &fields() const { return m_fields; }
  const map<string, weak_ptr<Manifold>> &manifolds() const {
    return m_manifolds;
  }
  const map<string, weak_ptr<TangentSpace>> &tangentspaces() const {
    return m_tangentspaces;
  }

  virtual bool invariant() const {
    return Common::invariant() && bool(project()) &&
           project()->configurations().count(name()) &&
           project()->configurations().at(name()).get() == this;
  }

  Configuration() = delete;
  Configuration(const Configuration &) = delete;
  Configuration(Configuration &&) = delete;
  Configuration &operator=(const Configuration &) = delete;
  Configuration &operator=(Configuration &&) = delete;

  friend class Project;
  Configuration(hidden, const string &name, const shared_ptr<Project> &project)
      : Common(name), m_project(project) {}
  Configuration(hidden) : Common(hidden()) {}

private:
  static shared_ptr<Configuration> create(const string &name,
                                          const shared_ptr<Project> &project) {
    return make_shared<Configuration>(hidden(), name, project);
  }
  static shared_ptr<Configuration> create(const H5::CommonFG &loc,
                                          const string &entry,
                                          const shared_ptr<Project> &project) {
    auto configuration = make_shared<Configuration>(hidden());
    configuration->read(loc, entry, project);
    return configuration;
  }
  void read(const H5::CommonFG &loc, const string &entry,
            const shared_ptr<Project> &project);

public:
  virtual ~Configuration() {}

  void merge(const shared_ptr<Configuration> &configuration);

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os, const Configuration &configuration) {
    return configuration.output(os);
  }
  virtual void write(const H5::CommonFG &loc,
                     const H5::H5Location &parent) const;

  void insertParameterValue(const shared_ptr<ParameterValue> &parametervalue);

private:
  friend class Basis;
  friend class CoordinateSystem;
  friend class DiscreteField;
  friend class Discretization;
  friend class Field;
  friend class Manifold;
  friend class TangentSpace;
  void insert(const string &name, const shared_ptr<Basis> &basis) {
    checked_emplace(m_bases, name, basis);
  }
  void insert(const string &name,
              const shared_ptr<CoordinateSystem> &coordinatesystem) {
    checked_emplace(m_coordinatesystems, name, coordinatesystem);
  }
  void insert(const string &name,
              const shared_ptr<DiscreteField> &discretefield) {
    checked_emplace(m_discretefields, name, discretefield);
  }
  void insert(const string &name,
              const shared_ptr<Discretization> &discretization) {
    checked_emplace(m_discretizations, name, discretization);
  }
  void insert(const string &name, const shared_ptr<Field> &field) {
    checked_emplace(m_fields, name, field);
  }
  void insert(const string &name, const shared_ptr<Manifold> &manifold) {
    checked_emplace(m_manifolds, name, manifold);
  }
  void insert(const string &name,
              const shared_ptr<TangentSpace> &tangentspace) {
    checked_emplace(m_tangentspaces, name, tangentspace);
  }
};
}

#define CONFIGURATION_HPP_DONE
#endif // #ifndef CONFIGURATION_HPP
#ifndef CONFIGURATION_HPP_DONE
#error "Cyclic include depencency"
#endif
