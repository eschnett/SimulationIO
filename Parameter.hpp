#ifndef PARAMETER_HPP
#define PARAMETER_HPP

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

class ParameterValue;

class Parameter : public Common,
                  public std::enable_shared_from_this<Parameter> {
  weak_ptr<Project> m_project;                               // parent
  map<string, shared_ptr<ParameterValue>> m_parametervalues; // children
  // type, range?, description?
public:
  shared_ptr<Project> project() const { return m_project.lock(); }
  const map<string, shared_ptr<ParameterValue>> &parametervalues() const {
    return m_parametervalues;
  }

  virtual bool invariant() const;

  Parameter() = delete;
  Parameter(const Parameter &) = delete;
  Parameter(Parameter &&) = delete;
  Parameter &operator=(const Parameter &) = delete;
  Parameter &operator=(Parameter &&) = delete;

  friend class Project;
  Parameter(hidden, const string &name, const shared_ptr<Project> &project)
      : Common(name), m_project(project) {}
  Parameter(hidden) : Common(hidden()) {}

private:
  static shared_ptr<Parameter> create(const string &name,
                                      const shared_ptr<Project> &project) {
    return make_shared<Parameter>(hidden(), name, project);
  }
  static shared_ptr<Parameter> create(const H5::H5Location &loc,
                                      const string &entry,
                                      const shared_ptr<Project> &project) {
    auto parameter = make_shared<Parameter>(hidden());
    parameter->read(loc, entry, project);
    return parameter;
  }
  void read(const H5::H5Location &loc, const string &entry,
            const shared_ptr<Project> &project);

public:
  virtual ~Parameter() {}

  void merge(const shared_ptr<Parameter> &parameter);

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os, const Parameter &parameter) {
    return parameter.output(os);
  }
  virtual void write(const H5::H5Location &loc,
                     const H5::H5Location &parent) const;

  shared_ptr<ParameterValue> createParameterValue(const string &name);
  shared_ptr<ParameterValue> getParameterValue(const string &name);
  shared_ptr<ParameterValue>
  copyParameterValue(const shared_ptr<ParameterValue> &parametervalue,
                     bool copy_children = false);
  shared_ptr<ParameterValue> readParameterValue(const H5::H5Location &loc,
                                                const string &entry);
};
} // namespace SimulationIO

#define PARAMETER_HPP_DONE
#endif // #ifndef PARAMETER_HPP
#ifndef PARAMETER_HPP_DONE
#error "Cyclic include depencency"
#endif
