#ifndef FIELD_HPP
#define FIELD_HPP

#include "Common.hpp"
#include "Manifold.hpp"
#include "Project.hpp"
#include "TangentSpace.hpp"
#include "TensorType.hpp"

#include <H5Cpp.h>

#include <cassert>
#include <iostream>
#include <map>
#include <string>

namespace SimulationIO {

using std::iostream;
using std::map;
using std::string;

struct Manifold;
struct TangentSpace;
struct TensorType;
struct DiscreteField;

struct Field : Common {
  Project *project;
  Manifold *manifold;
  TangentSpace *tangentspace;
  TensorType *tensortype;
  map<string, DiscreteField *> discretefields; // owned

  virtual bool invariant() const {
    bool inv =
        Common::invariant() && bool(project) && project->fields.count(name) &&
        project->fields.at(name) == this && bool(manifold) &&
        manifold->fields.count(name) && manifold->fields.at(name) == this &&
        bool(tangentspace) && tangentspace->fields.count(name) &&
        tangentspace->fields.at(name) == this && bool(tensortype) &&
        tangentspace->dimension == tensortype->dimension;
    for (const auto &df : discretefields)
      inv &= !df.first.empty() && bool(df.second);
    return inv;
  }

  Field() = delete;
  Field(const Field &) = delete;
  Field(Field &&) = delete;
  Field &operator=(const Field &) = delete;
  Field &operator=(Field &&) = delete;

private:
  friend class Project;
  Field(const string &name, Project *project, Manifold *manifold,
        TangentSpace *tangentspace, TensorType *tensortype)
      : Common(name), project(project), manifold(manifold),
        tangentspace(tangentspace), tensortype(tensortype) {
    manifold->insert(name, this);
    tangentspace->insert(name, this);
    // tensortypes->insert(this);
  }
  Field(const H5::CommonFG &loc, const string &entry, Project *project);

public:
  virtual ~Field() { assert(0); }

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os, const Field &field) {
    return field.output(os);
  }
  virtual void write(const H5::CommonFG &loc,
                     const H5::H5Location &parent) const;

  DiscreteField *createDiscreteField(const string &name,
                                     Discretization *discretization,
                                     Basis *basis);
  DiscreteField *createDiscreteField(const H5::CommonFG &loc,
                                     const string &entry);
};
}

#define FIELD_HPP_DONE
#endif // #ifndef FIELD_HPP
#ifndef FIELD_HPP_DONE
#error "Cyclic include depencency"
#endif
