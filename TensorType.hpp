#ifndef TENSORTYPE_HPP
#define TENSORTYPE_HPP

#include "Common.hpp"
#include "Helpers.hpp"
#include "Project.hpp"

#include <H5Cpp.h>

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

struct Field;
struct TensorComponent;

struct TensorType : Common, std::enable_shared_from_this<TensorType> {
  weak_ptr<Project> project; // parent
  int dimension;
  int rank;
  map<string, shared_ptr<TensorComponent>> tensorcomponents; // children
  map<int, shared_ptr<TensorComponent>> storage_indices;
  NoBackLink<weak_ptr<Field>> fields;

  virtual bool invariant() const {
    bool inv = Common::invariant() && bool(project.lock()) &&
               project.lock()->tensortypes.count(name) &&
               project.lock()->tensortypes.at(name).get() == this &&
               dimension >= 0 && rank >= 0 &&
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

  friend struct Project;
  TensorType(hidden, const string &name, const shared_ptr<Project> &project,
             int dimension, int rank)
      : Common(name), project(project), dimension(dimension), rank(rank) {}
  TensorType(hidden) : Common(hidden()) {}

private:
  static shared_ptr<TensorType> create(const string &name,
                                       const shared_ptr<Project> &project,
                                       int dimension, int rank) {
    return make_shared<TensorType>(hidden(), name, project, dimension, rank);
  }
  static shared_ptr<TensorType> create(const H5::CommonFG &loc,
                                       const string &entry,
                                       const shared_ptr<Project> &project) {
    auto tensortype = make_shared<TensorType>(hidden());
    tensortype->read(loc, entry, project);
    return tensortype;
  }
  void read(const H5::CommonFG &loc, const string &entry,
            const shared_ptr<Project> &project);

public:
  virtual ~TensorType() {}

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os, const TensorType &tensortype) {
    return tensortype.output(os);
  }
  virtual void write(const H5::CommonFG &loc,
                     const H5::H5Location &parent) const;

  shared_ptr<TensorComponent>
  createTensorComponent(const string &name, int storage_index,
                        const vector<int> &indexvalues);
  shared_ptr<TensorComponent> readTensorComponent(const H5::CommonFG &loc,
                                                  const string &entry);

private:
  friend struct Field;
  void noinsert(const shared_ptr<Field> &field) {}
};
}

#define TENSORTYPE_HPP_DONE
#endif // #ifndef TENSORTYPE_HPP
#ifndef TENSORTYPE_HPP_DONE
#error "Cyclic include depencency"
#endif
