#ifndef COMMON_HPP
#define COMMON_HPP

#include <H5Cpp.h>

#include <iostream>
#include <string>

namespace SimulationIO {

using std::ostream;
using std::string;

// Common to all file elements

struct Common {
  string name;

  Common(const string &name) : name(name) {}
  Common() {}
  virtual bool invariant() const { return !name.empty(); }

  virtual ~Common() {}
  virtual ostream &output(ostream &os, int level = 0) const = 0;
  virtual void write(const H5::CommonFG &loc,
                     const H5::H5Location &parent) const = 0;

  // The associations between names and integer values below MUST NOT BE
  // MODIFIED, except that new integer values may be added.
  enum types {
    type_Basis = 1,
    type_BasisVector = 2,
    type_DiscreteField = 3,
    type_DiscreteFieldBlock = 4,
    type_DiscreteFieldBlockData = 5,
    type_Discretization = 6,
    type_DiscretizationBlock = 7,
    type_Field = 8,
    type_Manifold = 9,
    type_Project = 10,
    type_TangentSpace = 11,
    type_TensorComponent = 12,
    type_TensorType = 13
  };
};
}

#endif // #ifndef COMMON_HPP
