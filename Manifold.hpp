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

struct CoordinateSystem;
struct Field;
struct CoordinateSystem;
struct Discretization;
struct SubDiscretization;

struct Manifold : Common, std::enable_shared_from_this<Manifold> {
  weak_ptr<Project> project;               // parent
  shared_ptr<Configuration> configuration; // with backlink
  int dimension;
  map<string, shared_ptr<Discretization>> discretizations;       // children
  map<string, shared_ptr<SubDiscretization>> subdiscretizations; // children
  map<string, weak_ptr<Field>> fields;                           // backlinks
  map<string, weak_ptr<CoordinateSystem>> coordinatesystems;     // backlinks

  virtual bool invariant() const {
    bool inv = Common::invariant() && bool(project.lock()) &&
               project.lock()->manifolds.count(name) &&
               project.lock()->manifolds.at(name).get() == this &&
               bool(configuration) && configuration->manifolds.count(name) &&
               configuration->manifolds.at(name).lock().get() == this &&
               dimension >= 0;
    for (const auto &d : discretizations)
      inv &= !d.first.empty() && bool(d.second);
    return inv;
  }

  Manifold() = delete;
  Manifold(const Manifold &) = delete;
  Manifold(Manifold &&) = delete;
  Manifold &operator=(const Manifold &) = delete;
  Manifold &operator=(Manifold &&) = delete;

  friend struct Project;
  Manifold(hidden, const string &name, const shared_ptr<Project> &project,
           const shared_ptr<Configuration> &configuration, int dimension)
      : Common(name), project(project), configuration(configuration),
        dimension(dimension) {}
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
  static shared_ptr<Manifold> create(const H5::CommonFG &loc,
                                     const string &entry,
                                     const shared_ptr<Project> &project) {
    auto manifold = make_shared<Manifold>(hidden());
    manifold->read(loc, entry, project);
    return manifold;
  }
  void read(const H5::CommonFG &loc, const string &entry,
            const shared_ptr<Project> &project);

public:
  virtual ~Manifold() {}

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os, const Manifold &manifold) {
    return manifold.output(os);
  }
  virtual void write(const H5::CommonFG &loc,
                     const H5::H5Location &parent) const;

  shared_ptr<Discretization>
  createDiscretization(const string &name,
                       const shared_ptr<Configuration> &configuration);
  shared_ptr<Discretization> createDiscretization(const H5::CommonFG &loc,
                                                  const string &entry);
  shared_ptr<SubDiscretization> createSubDiscretization(
      const string &name,
      const shared_ptr<Discretization> &parent_discretization,
      const shared_ptr<Discretization> &child_discretization,
      const vector<double> &factor, const vector<double> &offset);
  shared_ptr<SubDiscretization> createSubDiscretization(const H5::CommonFG &loc,
                                                        const string &entry);

private:
  friend struct CoordinateSystem;
  void insert(const string &name,
              const shared_ptr<CoordinateSystem> &coordinatesystem) {
    checked_emplace(coordinatesystems, name, coordinatesystem);
  }
  friend struct Field;
  void insert(const string &name, const shared_ptr<Field> &field) {
    checked_emplace(fields, name, field);
  }
};
}

#define MANIFOLD_HPP_DONE
#endif // #ifndef MANIFOLD_HPP
#ifndef MANIFOLD_HPP_DONE
#error "Cyclic include depencency"
#endif
