#ifndef CONFIGURATION_HPP
#define CONFIGURATION_HPP

#include "Common.hpp"
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

struct ParameterValue;

struct Configuration : Common, std::enable_shared_from_this<Configuration> {
  weak_ptr<Project> project;                               // parent
  map<string, shared_ptr<ParameterValue>> parametervalues; // links

  virtual bool invariant() const {
    return Common::invariant() && bool(project.lock()) &&
           project.lock()->configurations.count(name) &&
           project.lock()->configurations.at(name).get() == this;
  }

  Configuration() = delete;
  Configuration(const Configuration &) = delete;
  Configuration(Configuration &&) = delete;
  Configuration &operator=(const Configuration &) = delete;
  Configuration &operator=(Configuration &&) = delete;

  friend class Project;
  Configuration(hidden, const string &name, const shared_ptr<Project> &project)
      : Common(name), project(project) {}
  Configuration(hidden) : Common(hidden()) {}

private:
  static shared_ptr<Configuration> create(const string &name,
                                          const shared_ptr<Project> &project) {
    return make_shared<Configuration>(hidden(), name, project);
  }
  static shared_ptr<Configuration> create(const H5::CommonFG &loc,
                                          const string &entry,
                                          const shared_ptr<Project> &project) {
    auto configuration = make_shared<Configuration>(hidden());
    configuration->read(loc, entry, project);
    return configuration;
  }
  void read(const H5::CommonFG &loc, const string &entry,
            const shared_ptr<Project> &project);

public:
  virtual ~Configuration() {}

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os, const Configuration &configuration) {
    return configuration.output(os);
  }
  virtual void write(const H5::CommonFG &loc,
                     const H5::H5Location &parent) const;

  void insertParameterValue(const shared_ptr<ParameterValue> &parametervalue);
};
}

#define CONFIGURATION_HPP_DONE
#endif // #ifndef CONFIGURATION_HPP
#ifndef CONFIGURATION_HPP_DONE
#error "Cyclic include depencency"
#endif
