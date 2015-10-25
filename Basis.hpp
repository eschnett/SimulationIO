#ifndef BASIS_HPP
#define BASIS_HPP

#include "Common.hpp"
#include "Configuration.hpp"
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
using std::weak_ptr;

struct BasisVector;
struct DiscreteField;
// struct CoordinateBasis;

struct Basis : Common, std::enable_shared_from_this<Basis> {
  weak_ptr<TangentSpace> tangentspace;               // parent
  shared_ptr<Configuration> configuration;           // with backlink
  map<string, shared_ptr<BasisVector>> basisvectors; // children
  map<int, shared_ptr<BasisVector>> directions;
  NoBackLink<weak_ptr<DiscreteField>> discretefields;
  // map<string, CoordinateBasis *> coordinatebases;

  virtual bool invariant() const {
    return Common::invariant() && bool(tangentspace.lock()) &&
           tangentspace.lock()->bases.count(name) &&
           tangentspace.lock()->bases.at(name).get() == this &&
           bool(configuration) && configuration->bases.count(name) &&
           configuration->bases.at(name).lock().get() == this;
    // int(basisvectors.size()) == tangentspace->dimension
    // int(directions.size()) == tangentspace->dimension
  }

  Basis() = delete;
  Basis(const Basis &) = delete;
  Basis(Basis &&) = delete;
  Basis &operator=(const Basis &) = delete;
  Basis &operator=(Basis &&) = delete;

  friend struct TangentSpace;
  Basis(hidden, const string &name,
        const shared_ptr<TangentSpace> &tangentspace,
        const shared_ptr<Configuration> &configuration)
      : Common(name), tangentspace(tangentspace), configuration(configuration) {
  }
  Basis(hidden) : Common(hidden()) {}

private:
  static shared_ptr<Basis>
  create(const string &name, const shared_ptr<TangentSpace> &tangentspace,
         const shared_ptr<Configuration> &configuration) {
    auto basis =
        make_shared<Basis>(hidden(), name, tangentspace, configuration);
    configuration->insert(name, basis);
    return basis;
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

private:
  friend struct DiscreteField;
  void noinsert(const shared_ptr<DiscreteField> &discretefield) {}
};
}

#define BASIS_HPP_DONE
#endif // #ifndef BASIS_HPP
#ifndef BASIS_HPP_DONE
#error "Cyclic include depencency"
#endif
