#ifndef BASIS_HPP
#define BASIS_HPP

#include "Common.hpp"
#include "TangentSpace.hpp"

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

struct BasisVector;
struct DiscreteField;
// struct CoordinateBasis;

struct Basis : Common, std::enable_shared_from_this<Basis> {
  shared_ptr<TangentSpace> tangentspace;             // parent
  map<string, shared_ptr<BasisVector>> basisvectors; // children
  map<int, shared_ptr<BasisVector>> directions;
  NoBackLink<shared_ptr<DiscreteField>> discretefields;
  // map<string, CoordinateBasis *> coordinatebases;

  virtual bool invariant() const {
    return Common::invariant() && bool(tangentspace) &&
           tangentspace->bases.count(name) &&
           tangentspace->bases.at(name).get() == this;
    // int(basisvectors.size()) == tangentspace->dimension
    // int(directions.size()) == tangentspace->dimension
  }

  Basis() = delete;
  Basis(const Basis &) = delete;
  Basis(Basis &&) = delete;
  Basis &operator=(const Basis &) = delete;
  Basis &operator=(Basis &&) = delete;

  friend class TangentSpace;
  Basis(hidden, const string &name,
        const shared_ptr<TangentSpace> &tangentspace)
      : Common(name), tangentspace(tangentspace) {}
  Basis(hidden) : Common(hidden()) {}

private:
  static shared_ptr<Basis>
  create(const string &name, const shared_ptr<TangentSpace> &tangentspace) {
    return make_shared<Basis>(hidden(), name, tangentspace);
  }
  static shared_ptr<Basis>
  create(const H5::CommonFG &loc, const string &entry,
         const shared_ptr<TangentSpace> &tangentspace) {
    auto basis = make_shared<Basis>(hidden());
    basis->read(loc, entry, tangentspace);
    return basis;
  }
  void read(const H5::CommonFG &loc, const string &entry,
            const shared_ptr<TangentSpace> &tangentspace);

public:
  virtual ~Basis() {}

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os, const Basis &basis) {
    return basis.output(os);
  }
  virtual void write(const H5::CommonFG &loc,
                     const H5::H5Location &parent) const;

  shared_ptr<BasisVector> createBasisVector(const string &name, int direction);
  shared_ptr<BasisVector> createBasisVector(const H5::CommonFG &loc,
                                            const string &entry);

  void noinsert(const shared_ptr<DiscreteField> &discretefield) {}
};
}

#define BASIS_HPP_DONE
#endif // #ifndef BASIS_HPP
#ifndef BASIS_HPP_DONE
#error "Cyclic include depencency"
#endif
