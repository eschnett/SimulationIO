#ifndef MANIFOLD_HPP
#define MANIFOLD_HPP

#include "Common.hpp"
#include "Helpers.hpp"
#include "Project.hpp"

#include <H5Cpp.h>

#include <iostream>
#include <map>
#include <string>

namespace SimulationIO {

using std::ostream;
using std::map;
using std::string;

struct Field;
struct Discretization;

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

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os, const Manifold &manifold) {
    return manifold.output(os);
  }
  virtual void write(const H5::CommonFG &loc,
                     const H5::H5Location &parent) const;

  Discretization *createDiscretization(const string &name);
  Discretization *createDiscretization(const H5::CommonFG &loc,
                                       const string &entry);
  void insert(const string &name, Field *field) {
    checked_emplace(fields, name, field);
  }
};
}

#define MANIFOLD_HPP_DONE
#endif // #ifndef MANIFOLD_HPP
#ifndef MANIFOLD_HPP_DONE
#error "Cyclic include depencency"
#endif
