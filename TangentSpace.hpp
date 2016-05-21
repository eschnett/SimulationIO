#ifndef TANGENTSPACE_HPP
#define TANGENTSPACE_HPP

#include "Common.hpp"
#include "Configuration.hpp"
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

class Field;
class Basis;

class TangentSpace : public Common,
                     public std::enable_shared_from_this<TangentSpace> {
  map<string, weak_ptr<Field>> m_fields; // backlinks
public:
  weak_ptr<Project> project;               // parent
  shared_ptr<Configuration> configuration; // with backlink
  int dimension;
  map<string, shared_ptr<Basis>> bases; // children
  const map<string, weak_ptr<Field>> &fields() const { return m_fields; }

  virtual bool invariant() const {
    bool inv = Common::invariant() && bool(project.lock()) &&
               project.lock()->tangentspaces.count(name()) &&
               project.lock()->tangentspaces.at(name()).get() == this &&
               bool(configuration) &&
               configuration->tangentspaces().count(name()) &&
               configuration->tangentspaces().at(name()).lock().get() == this &&
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
               const shared_ptr<Configuration> &configuration, int dimension)
      : Common(name), project(project), configuration(configuration),
        dimension(dimension) {}
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

  shared_ptr<Basis> createBasis(const string &name,
                                const shared_ptr<Configuration> &configuration);
  shared_ptr<Basis> readBasis(const H5::CommonFG &loc, const string &entry);

private:
  friend class Field;
  void insert(const string &name, const shared_ptr<Field> &field) {
    checked_emplace(m_fields, name, field);
  }
};
}

#define TANGENTSPACE_HPP_DONE
#endif // #ifndef TANGENTSPACE_HPP
#ifndef TANGENTSPACE_HPP_DONE
#error "Cyclic include depencency"
#endif
