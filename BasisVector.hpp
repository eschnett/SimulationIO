#ifndef BASISVECTOR_HPP
#define BASISVECTOR_HPP

#include "Basis.hpp"
#include "Common.hpp"

#include <H5Cpp.h>

#include <iostream>
#include <memory>
#include <string>

using std::make_shared;
using std::ostream;
using std::shared_ptr;
using std::string;

namespace SimulationIO {

struct BasisVector : Common, std::enable_shared_from_this<BasisVector> {
  shared_ptr<Basis> basis; // parent
  int direction;

  virtual bool invariant() const {
    return Common::invariant() && bool(basis) &&
           basis->basisvectors.count(name) &&
           basis->basisvectors.at(name).get() == this && direction >= 0 &&
           direction < basis->tangentspace->dimension &&
           basis->directions.count(direction) &&
           basis->directions.at(direction).get() == this;
  }

  BasisVector() = delete;
  BasisVector(const BasisVector &) = delete;
  BasisVector(BasisVector &&) = delete;
  BasisVector &operator=(const BasisVector &) = delete;
  BasisVector &operator=(BasisVector &&) = delete;

  friend class Basis;
  BasisVector(hidden, const string &name, const shared_ptr<Basis> &basis,
              int direction)
      : Common(name), basis(basis), direction(direction) {}
  BasisVector(hidden) : Common(hidden()) {}

private:
  static shared_ptr<BasisVector>
  create(const string &name, const shared_ptr<Basis> &basis, int direction) {
    return make_shared<BasisVector>(hidden(), name, basis, direction);
  }
  static shared_ptr<BasisVector> create(const H5::CommonFG &loc,
                                        const string &entry,
                                        const shared_ptr<Basis> &basis) {
    auto basisvector = make_shared<BasisVector>(hidden());
    basisvector->read(loc, entry, basis);
    return basisvector;
  }
  void read(const H5::CommonFG &loc, const string &entry,
            const shared_ptr<Basis> &basis);

public:
  virtual ~BasisVector() {}

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
