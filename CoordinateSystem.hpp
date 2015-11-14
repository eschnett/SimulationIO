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

struct CoordinateField;

struct CoordinateSystem : Common,
                          std::enable_shared_from_this<CoordinateSystem> {
  weak_ptr<Project> project;                                 // parent
  shared_ptr<Configuration> configuration;                   // with backlink
  shared_ptr<Manifold> manifold;                             // with backlink
  map<string, shared_ptr<CoordinateField>> coordinatefields; // children
  map<int, shared_ptr<CoordinateField>> directions;
  // map<string, shared_ptr<CoordinateBasis>> coordinatebases;

  virtual bool invariant() const {
    return Common::invariant() && bool(project.lock()) &&
           project.lock()->coordinatesystems.count(name) &&
           project.lock()->coordinatesystems.at(name).get() == this &&
           bool(configuration) &&
           configuration->coordinatesystems.count(name) &&
           configuration->coordinatesystems.at(name).lock().get() == this &&
           bool(manifold) && manifold->coordinatesystems.count(name) &&
           manifold->coordinatesystems.at(name).lock().get() == this;
  }

  CoordinateSystem() = delete;
  CoordinateSystem(const CoordinateSystem &) = delete;
  CoordinateSystem(CoordinateSystem &&) = delete;
  CoordinateSystem &operator=(const CoordinateSystem &) = delete;
  CoordinateSystem &operator=(CoordinateSystem &&) = delete;

  friend struct Project;
  CoordinateSystem(hidden, const string &name,
                   const shared_ptr<Project> &project,
                   const shared_ptr<Configuration> &configuration,
                   const shared_ptr<Manifold> &manifold)
      : Common(name), project(project), configuration(configuration),
        manifold(manifold) {}
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
  create(const H5::CommonFG &loc, const string &entry,
         const shared_ptr<Project> &project) {
    auto coordinatesystem = make_shared<CoordinateSystem>(hidden());
    coordinatesystem->read(loc, entry, project);
    return coordinatesystem;
  }
  void read(const H5::CommonFG &loc, const string &entry,
            const shared_ptr<Project> &project);

public:
  virtual ~CoordinateSystem() {}

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os,
                             const CoordinateSystem &coordinatesystem) {
    return coordinatesystem.output(os);
  }
  virtual void write(const H5::CommonFG &loc,
                     const H5::H5Location &parent) const;

  shared_ptr<CoordinateField>
  createCoordinateField(const string &name, int direction,
                        const shared_ptr<Field> &field);
  shared_ptr<CoordinateField> readCoordinateField(const H5::CommonFG &loc,
                                                  const string &entry);
};
}

#define COORDINATESYSTEM_HPP_DONE
#endif // #ifndef COORDINATESYSTEM_HPP
#ifndef COORDINATESYSTEM_HPP_DONE
#error "Cyclic include depencency"
#endif
