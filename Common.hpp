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
};
}

#endif // #ifndef COMMON_HPP
