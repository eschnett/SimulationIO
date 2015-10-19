#ifndef TANGENTSPACE_HPP
#define TANGENTSPACE_HPP

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

struct Field;
struct Basis;

struct TangentSpace : Common, std::enable_shared_from_this<TangentSpace> {
  weak_ptr<Project> project; // parent
  int dimension;
  map<string, shared_ptr<Basis>> bases; // children
  map<string, weak_ptr<Field>> fields;  // backlinks

  virtual bool invariant() const {
    bool inv = Common::invariant() && bool(project.lock()) &&
               project.lock()->tangentspaces.count(name) &&
               project.lock()->tangentspaces.at(name).get() == this &&
               dimension >= 0;
    for (const auto &b : bases)
      inv &= !b.first.empty() && bool(b.second);
    return inv;
  }

  TangentSpace() = delete;
  TangentSpace(const TangentSpace &) = delete;
  TangentSpace(TangentSpace &&) = delete;
  TangentSpace &operator=(const TangentSpace &) = delete;
  TangentSpace &operator=(TangentSpace &&) = delete;

  friend class Project;
  TangentSpace(hidden, const string &name, const shared_ptr<Project> &project,
               int dimension)
      : Common(name), project(project), dimension(dimension) {}
  TangentSpace(hidden) : Common(hidden()) {}

private:
  static shared_ptr<TangentSpace> create(const string &name,
                                         const shared_ptr<Project> &project,
                                         int dimension) {
    return make_shared<TangentSpace>(hidden(), name, project, dimension);
  }
  static shared_ptr<TangentSpace> create(const H5::CommonFG &loc,
                                         const string &entry,
                                         const shared_ptr<Project> &project) {
    auto tangentspace = make_shared<TangentSpace>(hidden());
    tangentspace->read(loc, entry, project);
    return tangentspace;
  }
  void read(const H5::CommonFG &loc, const string &entry,
            const shared_ptr<Project> &project);

public:
  virtual ~TangentSpace() {}

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os, const TangentSpace &tangentspace) {
    return tangentspace.output(os);
  }
  virtual void write(const H5::CommonFG &loc,
                     const H5::H5Location &parent) const;

  shared_ptr<Basis> createBasis(const string &name);
  shared_ptr<Basis> createBasis(const H5::CommonFG &loc, const string &entry);

  void insert(const string &name, const shared_ptr<Field> &field) {
    checked_emplace(fields, name, field);
  }
};
}

#define TANGENTSPACE_HPP_DONE
#endif // #ifndef TANGENTSPACE_HPP
#ifndef TANGENTSPACE_HPP_DONE
#error "Cyclic include depencency"
#endif
