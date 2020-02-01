#ifndef MANIFOLD_HPP
#define MANIFOLD_HPP

#include "Common.hpp"
#include "Config.hpp"
#include "Configuration.hpp"
#include "Helpers.hpp"
#include "Project.hpp"

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
#include <asdf.hpp>
#endif

#ifdef SIMULATIONIO_HAVE_HDF5
#include <H5Cpp.h>
#endif

#ifdef SIMULATIONIO_HAVE_SILO
#include <silo.h>
#endif

#ifdef SIMULATIONIO_HAVE_TILEDB
#include <tiledb/tiledb>
#endif

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
  virtual string type() const { return "Manifold"; }

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

  virtual bool invariant() const;

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
#ifdef SIMULATIONIO_HAVE_HDF5
  static shared_ptr<Manifold> create(const H5::H5Location &loc,
                                     const string &entry,
                                     const shared_ptr<Project> &project) {
    auto manifold = make_shared<Manifold>(hidden());
    manifold->read(loc, entry, project);
    return manifold;
  }
  void read(const H5::H5Location &loc, const string &entry,
            const shared_ptr<Project> &project);
#endif
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
  static shared_ptr<Manifold> create(const shared_ptr<ASDF::reader_state> &rs,
                                     const YAML::Node &node,
                                     const shared_ptr<Project> &project) {
    auto manifold = make_shared<Manifold>(hidden());
    manifold->read(rs, node, project);
    return manifold;
  }
  void read(const shared_ptr<ASDF::reader_state> &rs, const YAML::Node &node,
            const shared_ptr<Project> &project);
#endif
#ifdef SIMULATIONIO_HAVE_SILO
  static shared_ptr<Manifold> create(const Silo<DBfile> &file,
                                     const string &loc,
                                     const shared_ptr<Project> &project) {
    auto manifold = make_shared<Manifold>(hidden());
    manifold->read(file, loc, project);
    return manifold;
  }
  void read(const Silo<DBfile> &file, const string &loc,
            const shared_ptr<Project> &project);
#endif

public:
  virtual ~Manifold() {}

  void merge(const shared_ptr<Manifold> &manifold);

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os, const Manifold &manifold) {
    return manifold.output(os);
  }
#ifdef SIMULATIONIO_HAVE_HDF5
  virtual void write(const H5::H5Location &loc,
                     const H5::H5Location &parent) const;
#endif
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
  virtual vector<string> yaml_path() const;
  ASDF::writer &write(ASDF::writer &w) const;
  friend ASDF::writer &operator<<(ASDF::writer &w, const Manifold &manifold) {
    return manifold.write(w);
  }
#endif
#ifdef SIMULATIONIO_HAVE_SILO
  virtual string silo_path() const;
  virtual void write(const Silo<DBfile> &file, const string &loc) const;
#endif
#ifdef SIMULATIONIO_HAVE_TILEDB
  virtual vector<string> tiledb_path() const;
  virtual void write(const tiledb::Context &ctx, const string &loc) const;
#endif

  shared_ptr<Discretization>
  createDiscretization(const string &name,
                       const shared_ptr<Configuration> &configuration);
  shared_ptr<Discretization>
  getDiscretization(const string &name,
                    const shared_ptr<Configuration> &configuration);
  shared_ptr<Discretization>
  copyDiscretization(const shared_ptr<Discretization> &discretization,
                     bool copy_children = false);
#ifdef SIMULATIONIO_HAVE_HDF5
  shared_ptr<Discretization> readDiscretization(const H5::H5Location &loc,
                                                const string &entry);
#endif
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
  shared_ptr<Discretization>
  readDiscretization(const shared_ptr<ASDF::reader_state> &rs,
                     const YAML::Node &node);
  shared_ptr<Discretization>
  getDiscretization(const shared_ptr<ASDF::reader_state> &rs,
                    const YAML::Node &node);
#endif
#ifdef SIMULATIONIO_HAVE_SILO
  shared_ptr<Discretization> readDiscretization(const Silo<DBfile> &file,
                                                const string &loc);
#endif
  shared_ptr<SubDiscretization> createSubDiscretization(
      const string &name,
      const shared_ptr<Discretization> &parent_discretization,
      const shared_ptr<Discretization> &child_discretization,
      const vector<double> &factor, const vector<double> &offset);
  shared_ptr<SubDiscretization>
  getSubDiscretization(const string &name,
                       const shared_ptr<Discretization> &parent_discretization,
                       const shared_ptr<Discretization> &child_discretization,
                       const vector<double> &factor,
                       const vector<double> &offset);
  shared_ptr<SubDiscretization>
  copySubDiscretization(const shared_ptr<SubDiscretization> &subdiscretization,
                        bool copy_children = false);
#ifdef SIMULATIONIO_HAVE_HDF5
  shared_ptr<SubDiscretization> readSubDiscretization(const H5::H5Location &loc,
                                                      const string &entry);
#endif
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
  shared_ptr<SubDiscretization>
  readSubDiscretization(const shared_ptr<ASDF::reader_state> &rs,
                        const YAML::Node &node);
#endif
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
  shared_ptr<SubDiscretization> readSubDiscretization(const Silo<DBfile> &file,
                                                      const string &loc);
#endif

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
