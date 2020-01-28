#ifndef TANGENTSPACE_HPP
#define TANGENTSPACE_HPP

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

namespace SimulationIO {

using std::make_shared;
using std::map;
using std::ostream;
using std::shared_ptr;
using std::string;
using std::weak_ptr;

class Field;
class Basis;

class TangentSpace : public Common,
                     public std::enable_shared_from_this<TangentSpace> {
  weak_ptr<Project> m_project;               // parent
  shared_ptr<Configuration> m_configuration; // with backlink
  int m_dimension;
  map<string, shared_ptr<Basis>> m_bases; // children
  map<string, weak_ptr<Field>> m_fields;  // backlinks
public:
  virtual string type() const { return "TangentSpace"; }

  shared_ptr<Project> project() const { return m_project.lock(); }
  shared_ptr<Configuration> configuration() const { return m_configuration; }
  int dimension() const { return m_dimension; }
  const map<string, shared_ptr<Basis>> &bases() const { return m_bases; }
  const map<string, weak_ptr<Field>> &fields() const { return m_fields; }

  virtual bool invariant() const;

  TangentSpace() = delete;
  TangentSpace(const TangentSpace &) = delete;
  TangentSpace(TangentSpace &&) = delete;
  TangentSpace &operator=(const TangentSpace &) = delete;
  TangentSpace &operator=(TangentSpace &&) = delete;

  friend class Project;
  TangentSpace(hidden, const string &name, const shared_ptr<Project> &project,
               const shared_ptr<Configuration> &configuration, int dimension)
      : Common(name), m_project(project), m_configuration(configuration),
        m_dimension(dimension) {}
  TangentSpace(hidden) : Common(hidden()) {}

private:
  static shared_ptr<TangentSpace>
  create(const string &name, const shared_ptr<Project> &project,
         const shared_ptr<Configuration> &configuration, int dimension) {
    auto tangentspace = make_shared<TangentSpace>(hidden(), name, project,
                                                  configuration, dimension);
    configuration->insert(name, tangentspace);
    return tangentspace;
  }
#ifdef SIMULATIONIO_HAVE_HDF5
  static shared_ptr<TangentSpace> create(const H5::H5Location &loc,
                                         const string &entry,
                                         const shared_ptr<Project> &project) {
    auto tangentspace = make_shared<TangentSpace>(hidden());
    tangentspace->read(loc, entry, project);
    return tangentspace;
  }
  void read(const H5::H5Location &loc, const string &entry,
            const shared_ptr<Project> &project);
#endif
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
  static shared_ptr<TangentSpace>
  create(const shared_ptr<ASDF::reader_state> &rs, const YAML::Node &node,
         const shared_ptr<Project> &project) {
    auto tangentspace = make_shared<TangentSpace>(hidden());
    tangentspace->read(rs, node, project);
    return tangentspace;
  }
  void read(const shared_ptr<ASDF::reader_state> &rs, const YAML::Node &node,
            const shared_ptr<Project> &project);
#endif

public:
  virtual ~TangentSpace() {}

  void merge(const shared_ptr<TangentSpace> &tangentspace);

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os, const TangentSpace &tangentspace) {
    return tangentspace.output(os);
  }
#ifdef SIMULATIONIO_HAVE_HDF5
  virtual void write(const H5::H5Location &loc,
                     const H5::H5Location &parent) const;
#endif
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
  virtual vector<string> yaml_path() const;
  ASDF::writer &write(ASDF::writer &w) const;
  friend ASDF::writer &operator<<(ASDF::writer &w,
                                  const TangentSpace &tangentspace) {
    return tangentspace.write(w);
  }
#endif
#ifdef SIMULATIONIO_HAVE_SILO
  virtual string silo_path() const;
  virtual void write(DBfile *file, const string &loc) const;
#endif
#ifdef SIMULATIONIO_HAVE_TILEDB
  virtual vector<string> tiledb_path() const;
  virtual void write(const tiledb::Context &ctx, const string &loc) const;
#endif

  shared_ptr<Basis> createBasis(const string &name,
                                const shared_ptr<Configuration> &configuration);
  shared_ptr<Basis> getBasis(const string &name,
                             const shared_ptr<Configuration> &configuration);
  shared_ptr<Basis> copyBasis(const shared_ptr<Basis> &basis,
                              bool copy_children = false);
#ifdef SIMULATIONIO_HAVE_HDF5
  shared_ptr<Basis> readBasis(const H5::H5Location &loc, const string &entry);
#endif
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
  shared_ptr<Basis> readBasis(const shared_ptr<ASDF::reader_state> &rs,
                              const YAML::Node &node);
  shared_ptr<Basis> getBasis(const shared_ptr<ASDF::reader_state> &rs,
                             const YAML::Node &node);
#endif

private:
  friend class Field;
  void insert(const string &name, const shared_ptr<Field> &field) {
    checked_emplace(m_fields, name, field, "TangentSpace", "fields");
  }
};

} // namespace SimulationIO

#define TANGENTSPACE_HPP_DONE
#endif // #ifndef TANGENTSPACE_HPP
#ifndef TANGENTSPACE_HPP_DONE
#error "Cyclic include depencency"
#endif
