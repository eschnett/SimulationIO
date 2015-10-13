#ifndef BASIS_HPP
#define BASIS_HPP

#include "Common.hpp"
#include "TangentSpace.hpp"

#include <H5Cpp.h>

#include <cassert>
#include <iostream>
#include <map>
#include <string>

namespace SimulationIO {

using std::ostream;
using std::map;
using std::string;

struct BasisVector;
struct CoordinateBasis;

struct Basis : Common {
  TangentSpace *tangentspace;
  map<string, BasisVector *> basisvectors; // owned
  map<int, BasisVector *> directions;
  map<string, CoordinateBasis *> coordinatebases;

  virtual bool invariant() const {
    return Common::invariant() && bool(tangentspace) &&
           tangentspace->bases.count(name) &&
           tangentspace->bases.at(name) == this &&
           int(basisvectors.size()) == tangentspace->dimension;
  }

  Basis() = delete;
  Basis(const Basis &) = delete;
  Basis(Basis &&) = delete;
  Basis &operator=(const Basis &) = delete;
  Basis &operator=(Basis &&) = delete;

private:
  friend class TangentSpace;
  Basis(const string &name, TangentSpace *tangentspace)
      : Common(name), tangentspace(tangentspace) {}
  Basis(const H5::CommonFG &loc, const string &entry,
        TangentSpace *tangentspace);

public:
  virtual ~Basis() { assert(0); }

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os, const Basis &basis) {
    return basis.output(os);
  }
  virtual void write(const H5::CommonFG &loc,
                     const H5::H5Location &parent) const;

  BasisVector *createBasisVector(const string &name, int direction);
  BasisVector *createBasisVector(const H5::CommonFG &loc, const string &entry);
};
}

#define BASIS_HPP_DONE
#endif // #ifndef BASIS_HPP
#ifndef BASIS_HPP_DONE
#error "Cyclic include depencency"
#endif
