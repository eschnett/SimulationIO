#ifndef MANIFOLD_HPP
#define MANIFOLD_HPP

#include "Common.hpp"
#include "Configuration.hpp"
#include "Helpers.hpp"
#include "Project.hpp"

#include <H5Cpp.h>

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
using std::weak_ptr;

class CoordinateSystem;
class Field;
class CoordinateSystem;
class Discretization;
class SubDiscretization;

class Manifold : public Common, public std::enable_shared_from_this<Manifold> {
  weak_ptr<Project> m_project;               // parent
  shared_ptr<Configuration> m_configuration; // with backlink
  int m_dimension;
  map<string, shared_ptr<Discretization>> m_discretizations;       // children
  map<string, shared_ptr<SubDiscretization>> m_subdiscretizations; // children
  map<string, weak_ptr<Field>> m_fields;                           // backlinks
  map<string, weak_ptr<CoordinateSystem>> m_coordinatesystems;     // backlinks
public:
  shared_ptr<Project> project() const { return m_project.lock(); }
  shared_ptr<Configuration> configuration() const { return m_configuration; }
  int dimension() const { return m_dimension; }
  const map<string, shared_ptr<Discretization>> &discretizations() const {
    return m_discretizations;
  }
  const map<string, shared_ptr<SubDiscretization>> &subdiscretizations() const {
    return m_subdiscretizations;
  }
  const map<string, weak_ptr<Field>> &fields() const { return m_fields; }
  const map<string, weak_ptr<CoordinateSystem>> &coordinatesystems() const {
    return m_coordinatesystems;
  }

  virtual bool invariant() const {
    bool inv = Common::invariant() && bool(project()) &&
               project()->manifolds().count(name()) &&
               project()->manifolds().at(name()).get() == this &&
               bool(configuration()) &&
               configuration()->manifolds().count(name()) &&
               configuration()->manifolds().at(name()).lock().get() == this &&
               dimension() >= 0;
    for (const auto &d : discretizations())
      inv &= !d.first.empty() && bool(d.second);
    return inv;
  }

  Manifold() = delete;
  Manifold(const Manifold &) = delete;
  Manifold(Manifold &&) = delete;
  Manifold &operator=(const Manifold &) = delete;
  Manifold &operator=(Manifold &&) = delete;

  friend class Project;
  Manifold(hidden, const string &name, const shared_ptr<Project> &project,
           const shared_ptr<Configuration> &configuration, int dimension)
      : Common(name), m_project(project), m_configuration(configuration),
        m_dimension(dimension) {}
  Manifold(hidden) : Common(hidden()) {}

private:
  static shared_ptr<Manifold>
  create(const string &name, const shared_ptr<Project> &project,
         const shared_ptr<Configuration> &configuration, int dimension) {
    auto manifold = make_shared<Manifold>(hidden(), name, project,
                                          configuration, dimension);
    configuration->insert(name, manifold);
    return manifold;
  }
  static shared_ptr<Manifold> create(const H5::H5Location &loc,
                                     const string &entry,
                                     const shared_ptr<Project> &project) {
    auto manifold = make_shared<Manifold>(hidden());
    manifold->read(loc, entry, project);
    return manifold;
  }
  void read(const H5::H5Location &loc, const string &entry,
            const shared_ptr<Project> &project);

public:
  virtual ~Manifold() {}

  void merge(const shared_ptr<Manifold> &manifold);

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os, const Manifold &manifold) {
    return manifold.output(os);
  }
  virtual void write(const H5::H5Location &loc,
                     const H5::H5Location &parent) const;

  shared_ptr<Discretization>
  createDiscretization(const string &name,
                       const shared_ptr<Configuration> &configuration);
  shared_ptr<Discretization> readDiscretization(const H5::H5Location &loc,
                                                const string &entry);
  shared_ptr<SubDiscretization> createSubDiscretization(
      const string &name,
      const shared_ptr<Discretization> &parent_discretization,
      const shared_ptr<Discretization> &child_discretization,
      const vector<double> &factor, const vector<double> &offset);
  shared_ptr<SubDiscretization> readSubDiscretization(const H5::H5Location &loc,
                                                      const string &entry);

private:
  friend class CoordinateSystem;
  void insert(const string &name,
              const shared_ptr<CoordinateSystem> &coordinatesystem) {
    checked_emplace(m_coordinatesystems, name, coordinatesystem, "Manifold",
                    "coordinatesystems");
  }
  friend class Field;
  void insert(const string &name, const shared_ptr<Field> &field) {
    checked_emplace(m_fields, name, field, "Manifold", "fields");
  }
};
} // namespace SimulationIO

#define MANIFOLD_HPP_DONE
#endif // #ifndef MANIFOLD_HPP
#ifndef MANIFOLD_HPP_DONE
#error "Cyclic include depencency"
#endif
