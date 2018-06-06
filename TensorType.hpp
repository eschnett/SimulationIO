#ifndef TENSORTYPE_HPP
#define TENSORTYPE_HPP

#include "Common.hpp"
#include "Config.hpp"
#include "Helpers.hpp"
#include "Project.hpp"

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
#include <asdf.hpp>
#endif

#ifdef SIMULATIONIO_HAVE_HDF5
#include <H5Cpp.h>
#endif

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

class Field;
class TensorComponent;

class TensorType : public Common,
                   public std::enable_shared_from_this<TensorType> {
  weak_ptr<Project> m_project; // parent
  int m_dimension;
  int m_rank;
  map<string, shared_ptr<TensorComponent>> m_tensorcomponents; // children
  map<int, shared_ptr<TensorComponent>> m_storage_indices;
  NoBackLink<weak_ptr<Field>> m_fields;

public:
  virtual string type() const { return "TensorType"; }

  shared_ptr<Project> project() const { return m_project.lock(); }
  int dimension() const { return m_dimension; }
  int rank() const { return m_rank; }
  const map<string, shared_ptr<TensorComponent>> &tensorcomponents() const {
    return m_tensorcomponents;
  }
  const map<int, shared_ptr<TensorComponent>> &storage_indices() const {
    return m_storage_indices;
  }
  NoBackLink<weak_ptr<Field>> fields() const { return m_fields; }

  virtual bool invariant() const;

  TensorType() = delete;
  TensorType(const TensorType &) = delete;
  TensorType(TensorType &&) = delete;
  TensorType &operator=(const TensorType &) = delete;
  TensorType &operator=(TensorType &&) = delete;

  friend class Project;
  TensorType(hidden, const string &name, const shared_ptr<Project> &project,
             int dimension, int rank)
      : Common(name), m_project(project), m_dimension(dimension), m_rank(rank) {
  }
  TensorType(hidden) : Common(hidden()) {}

private:
  static shared_ptr<TensorType> create(const string &name,
                                       const shared_ptr<Project> &project,
                                       int dimension, int rank) {
    return make_shared<TensorType>(hidden(), name, project, dimension, rank);
  }
#ifdef SIMULATIONIO_HAVE_HDF5
  static shared_ptr<TensorType> create(const H5::H5Location &loc,
                                       const string &entry,
                                       const shared_ptr<Project> &project) {
    auto tensortype = make_shared<TensorType>(hidden());
    tensortype->read(loc, entry, project);
    return tensortype;
  }
  void read(const H5::H5Location &loc, const string &entry,
            const shared_ptr<Project> &project);
#endif
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
  static shared_ptr<TensorType> create(const ASDF::reader_state &rs,
                                       const YAML::Node &node,
                                       const shared_ptr<Project> &project) {
    auto tensortype = make_shared<TensorType>(hidden());
    tensortype->read(rs, node, project);
    return tensortype;
  }
  void read(const ASDF::reader_state &rs, const YAML::Node &node,
            const shared_ptr<Project> &project);
#endif

public:
  virtual ~TensorType() {}

  void merge(const shared_ptr<TensorType> &tensortype);

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os, const TensorType &tensortype) {
    return tensortype.output(os);
  }
#ifdef SIMULATIONIO_HAVE_HDF5
  virtual void write(const H5::H5Location &loc,
                     const H5::H5Location &parent) const;
#endif
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
  virtual vector<string> yaml_path() const;
  ASDF::writer &write(ASDF::writer &w) const;
  friend ASDF::writer &operator<<(ASDF::writer &w,
                                  const TensorType &tensortype) {
    return tensortype.write(w);
  }
#endif

  shared_ptr<TensorComponent>
  createTensorComponent(const string &name, int storage_index,
                        const vector<int> &indexvalues);
  shared_ptr<TensorComponent>
  getTensorComponent(const string &name, int storage_index,
                     const vector<int> &indexvalues);
  shared_ptr<TensorComponent>
  copyTensorComponent(const shared_ptr<TensorComponent> &tensorcomponent,
                      bool copy_children = false);
#ifdef SIMULATIONIO_HAVE_HDF5
  shared_ptr<TensorComponent> readTensorComponent(const H5::H5Location &loc,
                                                  const string &entry);
#endif
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
  shared_ptr<TensorComponent> readTensorComponent(const ASDF::reader_state &rs,
                                                  const YAML::Node &node);
  shared_ptr<TensorComponent> getTensorComponent(const ASDF::reader_state &rs,
                                                 const YAML::Node &node);
#endif

private:
  friend class Field;
  void noinsert(const shared_ptr<Field> &field) {}
};

} // namespace SimulationIO

#define TENSORTYPE_HPP_DONE
#endif // #ifndef TENSORTYPE_HPP
#ifndef TENSORTYPE_HPP_DONE
#error "Cyclic include depencency"
#endif
