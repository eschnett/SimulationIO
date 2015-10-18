#ifndef PARAMETERVALUE_HPP
#define PARAMETERVALUE_HPP

#include "Common.hpp"
#include "Parameter.hpp"

#include <H5Cpp.h>

#include <cassert>
#include <iostream>
#include <map>
#include <string>

namespace SimulationIO {

using std::ostream;
using std::map;
using std::string;
using std::tuple;

struct Configuration;

struct ParameterValue : Common {
  Parameter *parameter;                        // parent
  map<string, Configuration *> configurations; // backlinks
  enum { type_empty, type_int, type_double, type_string } value_type;
  int value_int;
  double value_double;
  string value_string;

  virtual bool invariant() const {
    return Common::invariant() && bool(parameter) &&
           parameter->parametervalues.count(name) &&
           parameter->parametervalues.at(name) == this && bool(parameter) &&
           value_type >= type_empty && value_type <= type_string;
  }

  ParameterValue() = delete;
  ParameterValue(const ParameterValue &) = delete;
  ParameterValue(ParameterValue &&) = delete;
  ParameterValue &operator=(const ParameterValue &) = delete;
  ParameterValue &operator=(ParameterValue &&) = delete;

private:
  friend class Parameter;
  ParameterValue(const string &name, Parameter *parameter)
      : Common(name), parameter(parameter), value_type(type_empty) {}
  ParameterValue(const H5::CommonFG &loc, const string &entry,
                 Parameter *parameter);

public:
  virtual ~ParameterValue() { assert(0); }

  void setValue();
  void setValue(int i);
  void setValue(double d);
  void setValue(const string &s);

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os,
                             const ParameterValue &parametervalue) {
    return parametervalue.output(os);
  }
  virtual void write(const H5::CommonFG &loc,
                     const H5::H5Location &parent) const;

  void insert(Configuration *configuration);
};
}

#define PARAMETERVALUE_HPP_DONE
#endif // #ifndef PARAMETERVALUE_HPP
#ifndef PARAMETERVALUE_HPP_DONE
#error "Cyclic include depencency"
#endif
