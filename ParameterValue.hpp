#ifndef PARAMETERVALUE_HPP
#define PARAMETERVALUE_HPP

#include "Common.hpp"
#include "Configuration.hpp"

#include <H5Cpp.h>

#include <cassert>
#include <iostream>
#include <map>
#include <string>

namespace SimulationIO {

using std::ostream;
using std::map;
using std::string;

struct ParameterValue : Common {
  Configuration *configuration;
  Parameter *parameter;
  // value

  virtual bool invariant() const {
    return Common::invariant() && bool(configuration) &&
           configuration->parametervalues.count(name) &&
           configuration->parametervalues.at(name) == this;
  }

  ParameterValue() = delete;
  ParameterValue(const ParameterValue &) = delete;
  ParameterValue(ParameterValue &&) = delete;
  ParameterValue &operator=(const ParameterValue &) = delete;
  ParameterValue &operator=(ParameterValue &&) = delete;

private:
  friend class Configuration;
  ParameterValue(const string &name, Configuration *configuration,
                 Parameter *parameter)
      : Common(name), configuration(configuration), parameter(parameter) {}
  ParameterValue(const H5::CommonFG &loc, const string &entry,
                 Configuration *configuration);

public:
  virtual ~ParameterValue() { assert(0); }

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os, const ParameterValue &basis) {
    return basis.output(os);
  }
  virtual void write(const H5::CommonFG &loc,
                     const H5::H5Location &parent) const;
};
}

#define PARAMETERVALUE_HPP_DONE
#endif // #ifndef PARAMETERVALUE_HPP
#ifndef PARAMETERVALUE_HPP_DONE
#error "Cyclic include depencency"
#endif
