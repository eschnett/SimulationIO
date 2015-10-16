#ifndef PARAMETER_HPP
#define PARAMETER_HPP

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

struct Parameter : Common {
  Project *project;
  // type

  virtual bool invariant() const {
    return Common::invariant() && bool(project) &&
           project->parameters.count(name) &&
           project->parameters.at(name) == this;
  }

  Parameter() = delete;
  Parameter(const Parameter &) = delete;
  Parameter(Parameter &&) = delete;
  Parameter &operator=(const Parameter &) = delete;
  Parameter &operator=(Parameter &&) = delete;

private:
  friend class Project;
  Parameter(const string &name, Project *project)
      : Common(name), project(project) {}
  Parameter(const H5::CommonFG &loc, const string &entry, Project *project);

public:
  virtual ~Parameter() { assert(0); }

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os, const Parameter &parameter) {
    return parameter.output(os);
  }
  virtual void write(const H5::CommonFG &loc,
                     const H5::H5Location &parent) const;
};
}

#define PARAMETER_HPP_DONE
#endif // #ifndef PARAMETER_HPP
#ifndef PARAMETER_HPP_DONE
#error "Cyclic include depencency"
#endif
