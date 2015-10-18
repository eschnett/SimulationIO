#ifndef BASISVECTOR_HPP
#define BASISVECTOR_HPP

#include "Common.hpp"
#include "Basis.hpp"

#include <cassert>
#include <iostream>
#include <string>

namespace SimulationIO {

struct BasisVector : Common {
  Basis *basis; // parent
  int direction;

  virtual bool invariant() const {
    return Common::invariant() && bool(basis) &&
           basis->basisvectors.count(name) &&
           basis->basisvectors.at(name) == this && direction >= 0 &&
           direction < basis->tangentspace->dimension &&
           basis->directions.count(direction) &&
           basis->directions.at(direction) == this;
  }

  BasisVector() = delete;
  BasisVector(const BasisVector &) = delete;
  BasisVector(BasisVector &&) = delete;
  BasisVector &operator=(const BasisVector &) = delete;
  BasisVector &operator=(BasisVector &&) = delete;

private:
  friend class Basis;
  BasisVector(const string &name, Basis *basis, int direction)
      : Common(name), basis(basis), direction(direction) {}
  BasisVector(const H5::CommonFG &loc, const string &entry, Basis *basis);

public:
  virtual ~BasisVector() { assert(0); }

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os, const BasisVector &basisvector) {
    return basisvector.output(os);
  }
  virtual void write(const H5::CommonFG &loc,
                     const H5::H5Location &parent) const;
};
}

#define BASISVECTOR_HPP_DONE
#endif // #ifndef BASISVECTOR_HPP
#ifndef BASISVECTOR_HPP_DONE
#error "Cyclic include depencency"
#endif
