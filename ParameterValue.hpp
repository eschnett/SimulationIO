#ifndef PARAMETERVALUE_HPP
#define PARAMETERVALUE_HPP

#include "Common.hpp"
#include "Parameter.hpp"

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

struct Configuration;

struct ParameterValue : Common, std::enable_shared_from_this<ParameterValue> {
  weak_ptr<Parameter> parameter;                       // parent
  map<string, weak_ptr<Configuration>> configurations; // backlinks
  enum { type_empty, type_int, type_double, type_string } value_type;
  int value_int;
  double value_double;
  string value_string;

  virtual bool invariant() const {
    return Common::invariant() && bool(parameter.lock()) &&
           parameter.lock()->parametervalues.count(name) &&
           parameter.lock()->parametervalues.at(name).get() == this &&
           value_type >= type_empty && value_type <= type_string;
  }

  ParameterValue() = delete;
  ParameterValue(const ParameterValue &) = delete;
  ParameterValue(ParameterValue &&) = delete;
  ParameterValue &operator=(const ParameterValue &) = delete;
  ParameterValue &operator=(ParameterValue &&) = delete;

  friend struct Parameter;
  ParameterValue(hidden, const string &name,
                 const shared_ptr<Parameter> &parameter)
      : Common(name), parameter(parameter), value_type(type_empty) {}
  ParameterValue(hidden) : Common(hidden()) {}

private:
  static shared_ptr<ParameterValue>
  create(const string &name, const shared_ptr<Parameter> &parameter) {
    return make_shared<ParameterValue>(hidden(), name, parameter);
  }
  static shared_ptr<ParameterValue>
  create(const H5::CommonFG &loc, const string &entry,
         const shared_ptr<Parameter> &parameter) {
    auto parametervalue = make_shared<ParameterValue>(hidden());
    parametervalue->read(loc, entry, parameter);
    return parametervalue;
  }
  void read(const H5::CommonFG &loc, const string &entry,
            const shared_ptr<Parameter> &parameter);

public:
  virtual ~ParameterValue() {}

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

private:
  friend struct Configuration;
  void insert(const shared_ptr<Configuration> &configuration);
};
}

#define PARAMETERVALUE_HPP_DONE
#endif // #ifndef PARAMETERVALUE_HPP
#ifndef PARAMETERVALUE_HPP_DONE
#error "Cyclic include depencency"
#endif
