#ifndef TANGENTSPACE_HPP
#define TANGENTSPACE_HPP

#include "Common.hpp"
#include "Helpers.hpp"
#include "Project.hpp"

#include <H5Cpp.h>

#include <iostream>
#include <map>
#include <string>

namespace SimulationIO {

using std::iostream;
using std::map;
using std::string;

struct Field;
struct Basis;

struct TangentSpace : Common {
  Project *project;
  int dimension;
  map<string, Basis *> bases; // owned
  map<string, Field *> fields;

  virtual bool invariant() const {
    bool inv = Common::invariant() && bool(project) &&
               project->tangentspaces.count(name) &&
               project->tangentspaces.at(name) == this && dimension >= 0;
    for (const auto &b : bases)
      inv &= !b.first.empty() && bool(b.second);
    for (const auto &f : fields)
      inv &= !f.first.empty() && bool(f.second);
    return inv;
  }

  TangentSpace() = delete;
  TangentSpace(const TangentSpace &) = delete;
  TangentSpace(TangentSpace &&) = delete;
  TangentSpace &operator=(const TangentSpace &) = delete;
  TangentSpace &operator=(TangentSpace &&) = delete;

private:
  friend class Project;
  TangentSpace(const string &name, Project *project, int dimension)
      : Common(name), project(project), dimension(dimension) {}
  TangentSpace(const H5::CommonFG &loc, const string &entry, Project *project);

public:
  virtual ~TangentSpace() { assert(0); }

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os, const TangentSpace &tangentspace) {
    return tangentspace.output(os);
  }
  virtual void write(const H5::CommonFG &loc,
                     const H5::H5Location &parent) const;

  void insert(const string &name, Field *field) {
    checked_emplace(fields, name, field);
  }
};
}

#define TANGENTSPACE_HPP_DONE
#endif // #ifndef TANGENTSPACE_HPP
#ifndef TANGENTSPACE_HPP_DONE
#error "Cyclic include depencency"
#endif