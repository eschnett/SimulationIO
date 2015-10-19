#ifndef CONFIGURATION_HPP
#define CONFIGURATION_HPP

#include "Common.hpp"
#include "Helpers.hpp"
#include "Project.hpp"

#include <H5Cpp.h>

#include <iostream>
#include <map>
#include <string>

namespace SimulationIO {

using std::iostream;
using std::map;
using std::string;

struct ParameterValue;

struct Configuration : Common {
  Project *project;                              // parent
  map<string, ParameterValue *> parametervalues; // links

  virtual bool invariant() const {
    return Common::invariant() && bool(project) &&
           project->configurations.count(name) &&
           project->configurations.at(name) == this;
  }

  Configuration() = delete;
  Configuration(const Configuration &) = delete;
  Configuration(Configuration &&) = delete;
  Configuration &operator=(const Configuration &) = delete;
  Configuration &operator=(Configuration &&) = delete;

private:
  friend class Project;
  Configuration(const string &name, Project *project)
      : Common(name), project(project) {}
  Configuration(const H5::CommonFG &loc, const string &entry, Project *project);

public:
  virtual ~Configuration() { assert(0); }

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os, const Configuration &configuration) {
    return configuration.output(os);
  }
  virtual void write(const H5::CommonFG &loc,
                     const H5::H5Location &parent) const;

  void insertParameterValue(ParameterValue *parametervalue);
};
}

#define CONFIGURATION_HPP_DONE
#endif // #ifndef CONFIGURATION_HPP
#ifndef CONFIGURATION_HPP_DONE
#error "Cyclic include depencency"
#endif
