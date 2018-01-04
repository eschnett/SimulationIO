#ifndef COORDINATESYSTEM_HPP
#define COORDINATESYSTEM_HPP

#include "Common.hpp"
#include "Configuration.hpp"
#include "Manifold.hpp"

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

class CoordinateField;

class CoordinateSystem : public Common,
                         public std::enable_shared_from_this<CoordinateSystem> {
  weak_ptr<Project> m_project;                                 // parent
  shared_ptr<Configuration> m_configuration;                   // with backlink
  shared_ptr<Manifold> m_manifold;                             // with backlink
  map<string, shared_ptr<CoordinateField>> m_coordinatefields; // children
  map<int, shared_ptr<CoordinateField>> m_directions;
  // map<string, shared_ptr<CoordinateBasis>> m_coordinatebases;
public:
  shared_ptr<Project> project() const { return m_project.lock(); }
  shared_ptr<Configuration> configuration() const { return m_configuration; }
  shared_ptr<Manifold> manifold() const { return m_manifold; }
  const map<string, shared_ptr<CoordinateField>> &coordinatefields() const {
    return m_coordinatefields;
  }
  const map<int, shared_ptr<CoordinateField>> &directions() const {
    return m_directions;
  }

  virtual bool invariant() const {
    return Common::invariant() && bool(project()) &&
           project()->coordinatesystems().count(name()) &&
           project()->coordinatesystems().at(name()).get() == this &&
           bool(configuration()) &&
           configuration()->coordinatesystems().count(name()) &&
           configuration()->coordinatesystems().at(name()).lock().get() ==
               this &&
           bool(manifold()) && manifold()->coordinatesystems().count(name()) &&
           manifold()->coordinatesystems().at(name()).lock().get() == this;
  }

  CoordinateSystem() = delete;
  CoordinateSystem(const CoordinateSystem &) = delete;
  CoordinateSystem(CoordinateSystem &&) = delete;
  CoordinateSystem &operator=(const CoordinateSystem &) = delete;
  CoordinateSystem &operator=(CoordinateSystem &&) = delete;

  friend class Project;
  CoordinateSystem(hidden, const string &name,
                   const shared_ptr<Project> &project,
                   const shared_ptr<Configuration> &configuration,
                   const shared_ptr<Manifold> &manifold)
      : Common(name), m_project(project), m_configuration(configuration),
        m_manifold(manifold) {}
  CoordinateSystem(hidden) : Common(hidden()) {}

private:
  static shared_ptr<CoordinateSystem>
  create(const string &name, const shared_ptr<Project> &project,
         const shared_ptr<Configuration> &configuration,
         const shared_ptr<Manifold> &manifold) {
    auto coordinatesystem = make_shared<CoordinateSystem>(
        hidden(), name, project, configuration, manifold);
    configuration->insert(name, coordinatesystem);
    manifold->insert(name, coordinatesystem);
    return coordinatesystem;
  }
  static shared_ptr<CoordinateSystem>
  create(const H5::H5Location &loc, const string &entry,
         const shared_ptr<Project> &project) {
    auto coordinatesystem = make_shared<CoordinateSystem>(hidden());
    coordinatesystem->read(loc, entry, project);
    return coordinatesystem;
  }
  void read(const H5::H5Location &loc, const string &entry,
            const shared_ptr<Project> &project);

public:
  virtual ~CoordinateSystem() {}

  void merge(const shared_ptr<CoordinateSystem> &coordinatesystem);

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os,
                             const CoordinateSystem &coordinatesystem) {
    return coordinatesystem.output(os);
  }
  virtual void write(const H5::H5Location &loc,
                     const H5::H5Location &parent) const;

  shared_ptr<CoordinateField>
  createCoordinateField(const string &name, int direction,
                        const shared_ptr<Field> &field);
  shared_ptr<CoordinateField> readCoordinateField(const H5::H5Location &loc,
                                                  const string &entry);
};
} // namespace SimulationIO

#define COORDINATESYSTEM_HPP_DONE
#endif // #ifndef COORDINATESYSTEM_HPP
#ifndef COORDINATESYSTEM_HPP_DONE
#error "Cyclic include depencency"
#endif
