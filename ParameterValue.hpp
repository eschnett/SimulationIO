#ifndef PARAMETERVALUE_HPP
#define PARAMETERVALUE_HPP

#include "Common.hpp"
#include "Parameter.hpp"

#include <asdf.hpp>

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

class Configuration;

class ParameterValue : public Common,
                       public std::enable_shared_from_this<ParameterValue> {
  weak_ptr<Parameter> m_parameter;                       // parent
  map<string, weak_ptr<Configuration>> m_configurations; // backlinks
public:
  virtual string type() const { return "ParameterValue"; }

  shared_ptr<Parameter> parameter() const { return m_parameter.lock(); }
  const map<string, weak_ptr<Configuration>> &configurations() const {
    return m_configurations;
  }
  enum value_type_t {
    type_empty,
    type_int,
    type_double,
    type_string,
  } value_type;
  long long value_int;
  double value_double;
  string value_string;

  virtual bool invariant() const;

  ParameterValue() = delete;
  ParameterValue(const ParameterValue &) = delete;
  ParameterValue(ParameterValue &&) = delete;
  ParameterValue &operator=(const ParameterValue &) = delete;
  ParameterValue &operator=(ParameterValue &&) = delete;

  friend class Parameter;
  ParameterValue(hidden, const string &name,
                 const shared_ptr<Parameter> &parameter)
      : Common(name), m_parameter(parameter), value_type(type_empty) {}
  ParameterValue(hidden) : Common(hidden()) {}

private:
  static shared_ptr<ParameterValue>
  create(const string &name, const shared_ptr<Parameter> &parameter) {
    return make_shared<ParameterValue>(hidden(), name, parameter);
  }
  static shared_ptr<ParameterValue>
  create(const H5::H5Location &loc, const string &entry,
         const shared_ptr<Parameter> &parameter) {
    auto parametervalue = make_shared<ParameterValue>(hidden());
    parametervalue->read(loc, entry, parameter);
    return parametervalue;
  }
  static shared_ptr<ParameterValue>
  create(const ASDF::reader_state &rs, const YAML::Node &node,
         const shared_ptr<Parameter> &parameter) {
    auto parametervalue = make_shared<ParameterValue>(hidden());
    parametervalue->read(rs, node, parameter);
    return parametervalue;
  }
  void read(const H5::H5Location &loc, const string &entry,
            const shared_ptr<Parameter> &parameter);
  void read(const ASDF::reader_state &rs, const YAML::Node &node,
            const shared_ptr<Parameter> &parameter);

public:
  virtual ~ParameterValue() {}

  void merge(const shared_ptr<ParameterValue> &parametervalue);

  void setValue();
  void setValue(long long i);
  void setValue(int i) { setValue((long long)i); }
  void setValue(long i) { setValue((long long)i); }
  void setValue(double d);
  void setValue(const string &s);

  value_type_t getValueType() const;
  long long getValueInt() const;
  double getValueDouble() const;
  string getValueString() const;

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os,
                             const ParameterValue &parametervalue) {
    return parametervalue.output(os);
  }
  virtual void write(const H5::H5Location &loc,
                     const H5::H5Location &parent) const;
  virtual string yaml_alias() const;
  ASDF::writer &write(ASDF::writer &w) const;
  friend ASDF::writer &operator<<(ASDF::writer &w,
                                  const ParameterValue &parametervalue) {
    return parametervalue.write(w);
  }

private:
  friend class Configuration;
  void insert(const shared_ptr<Configuration> &configuration);
};

} // namespace SimulationIO

#define PARAMETERVALUE_HPP_DONE
#endif // #ifndef PARAMETERVALUE_HPP
#ifndef PARAMETERVALUE_HPP_DONE
#error "Cyclic include depencency"
#endif
