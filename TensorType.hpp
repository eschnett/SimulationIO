#ifndef TENSORTYPE_HPP
#define TENSORTYPE_HPP

#include "Common.hpp"
#include "Helpers.hpp"
#include "Project.hpp"

#include <H5Cpp.h>

#include <cassert>
#include <map>
#include <string>
#include <vector>

namespace SimulationIO {

using std::map;
using std::string;
using std::vector;

struct TensorComponent;

struct TensorType : Common {
  Project *project;
  int dimension;
  int rank;
  map<string, TensorComponent *> tensorcomponents; // owned

  virtual bool invariant() const {
    bool inv = Common::invariant() && bool(project) &&
               project->tensortypes.count(name) &&
               project->tensortypes.at(name) == this && dimension >= 0 &&
               rank >= 0 &&
               int(tensorcomponents.size()) <= ipow(dimension, rank);
    for (const auto &tc : tensorcomponents)
      inv &= !tc.first.empty() && bool(tc.second);
    return inv;
  }

  TensorType() = delete;
  TensorType(const TensorType &) = delete;
  TensorType(TensorType &&) = delete;
  TensorType &operator=(const TensorType &) = delete;
  TensorType &operator=(TensorType &&) = delete;

private:
  friend class Project;
  TensorType(const string &name, Project *project, int dimension, int rank)
      : Common(name), project(project), dimension(dimension), rank(rank) {}
  TensorType(const H5::CommonFG &loc, const string &entry, Project *project);

public:
  virtual ~TensorType() { assert(0); }

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os, const TensorType &tensortype) {
    return tensortype.output(os);
  }
  virtual void write(const H5::CommonFG &loc,
                     const H5::H5Location &parent) const;

  TensorComponent *createTensorComponent(const string &name,
                                         const vector<int> &indexvalues);
  TensorComponent *createTensorComponent(const H5::CommonFG &loc,
                                         const string &entry);
};
}

#define TENSORTYPE_HPP_DONE
#endif // #ifndef TENSORTYPE_HPP
#ifndef TENSORTYPE_HPP_DONE
#error "Cyclic include depencency"
#endif
