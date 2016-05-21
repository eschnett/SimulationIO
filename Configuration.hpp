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

struct Basis;
struct CoordinateSystem;
class DiscreteField;
struct Discretization;
struct Field;
class Manifold;
struct ParameterValue;
class TangentSpace;

class Configuration : public Common,
                      public std::enable_shared_from_this<Configuration> {
  map<string, lazy_weak_ptr<Field>> m_fields; // backlinks
public:
  weak_ptr<Project> project;                                 // parent
  map<string, shared_ptr<ParameterValue>> parametervalues;   // links
  map<string, weak_ptr<Basis>> bases;                        // backlinks
  map<string, weak_ptr<CoordinateSystem>> coordinatesystems; // backlinks
  map<string, weak_ptr<DiscreteField>> discretefields;       // backlinks
  map<string, weak_ptr<Discretization>> discretizations;     // backlinks
  const map<string, lazy_weak_ptr<Field>> &fields() const { return m_fields; }
  map<string, weak_ptr<Manifold>> manifolds;         // backlinks
  map<string, weak_ptr<TangentSpace>> tangentspaces; // backlinks

  virtual bool invariant() const {
    return Common::invariant() && bool(project.lock()) &&
           project.lock()->configurations.count(name) &&
           project.lock()->configurations.at(name).get() == this;
  }

  Configuration() = delete;
  Configuration(const Configuration &) = delete;
  Configuration(Configuration &&) = delete;
  Configuration &operator=(const Configuration &) = delete;
  Configuration &operator=(Configuration &&) = delete;

  friend struct Project;
  Configuration(hidden, const string &name, const shared_ptr<Project> &project)
      : Common(name), project(project) {}
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

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os, const Configuration &configuration) {
    return configuration.output(os);
  }
  virtual void write(const H5::CommonFG &loc,
                     const H5::H5Location &parent) const;

  void insertParameterValue(const shared_ptr<ParameterValue> &parametervalue);

private:
  friend struct Basis;
  friend struct CoordinateSystem;
  friend class DiscreteField;
  friend struct Discretization;
  friend struct Field;
  friend class Manifold;
  friend class TangentSpace;
  void insert(const string &name, const shared_ptr<Basis> &basis) {
    checked_emplace(bases, name, basis);
  }
  void insert(const string &name,
              const shared_ptr<CoordinateSystem> &coordinatesystem) {
    checked_emplace(coordinatesystems, name, coordinatesystem);
  }
  void insert(const string &name,
              const shared_ptr<DiscreteField> &discretefield) {
    checked_emplace(discretefields, name, discretefield);
  }
  void insert(const string &name,
              const shared_ptr<Discretization> &discretization) {
    checked_emplace(discretizations, name, discretization);
  }
  void insert(const string &name, const lazy_ptr<Field> &field) {
    checked_emplace(m_fields, name, field);
  }
  void insert(const string &name, const shared_ptr<Manifold> &manifold) {
    checked_emplace(manifolds, name, manifold);
  }
  void insert(const string &name,
              const shared_ptr<TangentSpace> &tangentspace) {
    checked_emplace(tangentspaces, name, tangentspace);
  }
};
}

#define CONFIGURATION_HPP_DONE
#endif // #ifndef CONFIGURATION_HPP
#ifndef CONFIGURATION_HPP_DONE
#error "Cyclic include depencency"
#endif
