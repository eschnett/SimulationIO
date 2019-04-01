#ifndef SUBDISCRETIZATION_HPP
#define SUBDISCRETIZATION_HPP

#include "Common.hpp"
#include "Config.hpp"
#include "Discretization.hpp"
#include "Manifold.hpp"

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
#include <asdf.hpp>
#endif

#ifdef SIMULATIONIO_HAVE_HDF5
#include <H5Cpp.h>
#endif

#ifdef SIMULATIONIO_HAVE_TILEDB
#include <tiledb/tiledb>
#endif

#include <cmath>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace SimulationIO {

using std::make_shared;
using std::map;
using std::ostream;
using std::pair;
using std::shared_ptr;
using std::string;
using std::vector;
using std::weak_ptr;

class SubDiscretization
    : public Common,
      public std::enable_shared_from_this<SubDiscretization> {
  weak_ptr<Manifold> m_manifold;
  shared_ptr<Discretization> m_parent_discretization;
  shared_ptr<Discretization> m_child_discretization;
  vector<double> m_factor;
  vector<double> m_offset;
  // child_idx = factor * parent_idx - offset
  // AMR refinement: factor=2, offset=0
  // cell from vertex: factor=1, offset=1/2

public:
  virtual string type() const { return "SubDiscretization"; }

  shared_ptr<Manifold> manifold() const { return m_manifold.lock(); }
  shared_ptr<Discretization> parent_discretization() const {
    return m_parent_discretization;
  }
  shared_ptr<Discretization> child_discretization() const {
    return m_child_discretization;
  }
  vector<double> factor() const { return m_factor; }
  vector<double> offset() const { return m_offset; }

  vector<double> child2parent(const vector<double> &child_idx) const {
    vector<double> parent_idx(child_idx.size());
    for (int d = 0; d < int(parent_idx.size()); ++d)
      parent_idx.at(d) = (child_idx.at(d) + offset().at(d)) / factor().at(d);
    return parent_idx;
  }
  vector<double> parent2child(const vector<double> &parent_idx) const {
    vector<double> child_idx(parent_idx.size());
    for (int d = 0; d < int(child_idx.size()); ++d)
      child_idx.at(d) = factor().at(d) * child_idx.at(d) - offset().at(d);
    return child_idx;
  }

  virtual bool invariant() const;

  SubDiscretization() = delete;
  SubDiscretization(const SubDiscretization &) = delete;
  SubDiscretization(SubDiscretization &&) = delete;
  SubDiscretization &operator=(const SubDiscretization &) = delete;
  SubDiscretization &operator=(SubDiscretization &&) = delete;

  friend class Manifold;
  SubDiscretization(hidden, const string &name,
                    const shared_ptr<Manifold> &manifold,
                    const shared_ptr<Discretization> &parent_discretization,
                    const shared_ptr<Discretization> &child_discretization,
                    const vector<double> &factor, const vector<double> &offset)
      : Common(name), m_manifold(manifold),
        m_parent_discretization(parent_discretization),
        m_child_discretization(child_discretization), m_factor(factor),
        m_offset(offset) {}
  SubDiscretization(hidden) : Common(hidden()) {}

private:
  static shared_ptr<SubDiscretization>
  create(const string &name, const shared_ptr<Manifold> &manifold,
         const shared_ptr<Discretization> &parent_discretization,
         const shared_ptr<Discretization> &child_discretization,
         const vector<double> &factor, const vector<double> &offset) {
    auto subdiscretization = make_shared<SubDiscretization>(
        hidden(), name, manifold, parent_discretization, child_discretization,
        factor, offset);
    parent_discretization->insertChild(name, subdiscretization);
    child_discretization->insertParent(name, subdiscretization);
    return subdiscretization;
  }
#ifdef SIMULATIONIO_HAVE_HDF5
  static shared_ptr<SubDiscretization>
  create(const H5::H5Location &loc, const string &entry,
         const shared_ptr<Manifold> &manifold) {
    auto subdiscretization = make_shared<SubDiscretization>(hidden());
    subdiscretization->read(loc, entry, manifold);
    return subdiscretization;
  }
  void read(const H5::H5Location &loc, const string &entry,
            const shared_ptr<Manifold> &manifold);
#endif
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
  static shared_ptr<SubDiscretization>
  create(const shared_ptr<ASDF::reader_state> &rs, const YAML::Node &node,
         const shared_ptr<Manifold> &manifold) {
    auto subdiscretization = make_shared<SubDiscretization>(hidden());
    subdiscretization->read(rs, node, manifold);
    return subdiscretization;
  }
  void read(const shared_ptr<ASDF::reader_state> &rs, const YAML::Node &node,
            const shared_ptr<Manifold> &manifold);
#endif

public:
  virtual ~SubDiscretization() {}

  void merge(const shared_ptr<SubDiscretization> &subdiscretization);

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os,
                             const SubDiscretization &subdiscretization) {
    return subdiscretization.output(os);
  }
#ifdef SIMULATIONIO_HAVE_HDF5
  virtual void write(const H5::H5Location &loc,
                     const H5::H5Location &parent) const;
#endif
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
  virtual vector<string> yaml_path() const;
  ASDF::writer &write(ASDF::writer &w) const;
  friend ASDF::writer &operator<<(ASDF::writer &w,
                                  const SubDiscretization &subdiscretization) {
    return subdiscretization.write(w);
  }
#endif
#ifdef SIMULATIONIO_HAVE_TILEDB
  virtual vector<string> tiledb_path() const;
  virtual void write(const tiledb::Context &ctx, const string &loc) const;
#endif
};

} // namespace SimulationIO

#define SUBDISCRETIZATION_HPP_DONE
#endif // #ifndef SUBDISCRETIZATION_HPP
#ifndef SUBDISCRETIZATION_HPP_DONE
#error "Cyclic include depencency"
#endif
