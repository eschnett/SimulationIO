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

class BasisVector;
class DiscreteField;
// class CoordinateBasis;

class Basis : public Common, public std::enable_shared_from_this<Basis> {
  weak_ptr<TangentSpace> m_tangentspace;               // parent
  shared_ptr<Configuration> m_configuration;           // with backlink
  map<string, shared_ptr<BasisVector>> m_basisvectors; // children
  map<int, shared_ptr<BasisVector>> m_directions;
  NoBackLink<weak_ptr<DiscreteField>> m_discretefields;
  // map<string, CoordinateBasis *> m_coordinatebases;
public:
  shared_ptr<TangentSpace> tangentspace() const {
    return m_tangentspace.lock();
  }
  shared_ptr<Configuration> configuration() const { return m_configuration; }
  const map<string, shared_ptr<BasisVector>> &basisvectors() const {
    return m_basisvectors;
  }
  const map<int, shared_ptr<BasisVector>> &directions() const {
    return m_directions;
  }
  NoBackLink<weak_ptr<DiscreteField>> discretefields() const {
    return m_discretefields;
  }

  virtual bool invariant() const {
    return Common::invariant() && bool(tangentspace()) &&
           tangentspace()->bases().count(name()) &&
           tangentspace()->bases().at(name()).get() == this &&
           bool(configuration()) && configuration()->bases().count(name()) &&
           configuration()->bases().at(name()).lock().get() == this;
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
        const shared_ptr<TangentSpace> &tangentspace,
        const shared_ptr<Configuration> &configuration)
      : Common(name), m_tangentspace(tangentspace),
        m_configuration(configuration) {}
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
  create(const H5::H5Location &loc, const string &entry,
         const shared_ptr<TangentSpace> &tangentspace) {
    auto basis = make_shared<Basis>(hidden());
    basis->read(loc, entry, tangentspace);
    return basis;
  }
  void read(const H5::H5Location &loc, const string &entry,
            const shared_ptr<TangentSpace> &tangentspace);

public:
  virtual ~Basis() {}

  void merge(const shared_ptr<Basis> &basis);

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os, const Basis &basis) {
    return basis.output(os);
  }
  virtual void write(const H5::H5Location &loc,
                     const H5::H5Location &parent) const;

  shared_ptr<BasisVector> createBasisVector(const string &name, int direction);
  shared_ptr<BasisVector> getBasisVector(const string &name, int direction);
  shared_ptr<BasisVector>
  copyBasisVector(const shared_ptr<BasisVector> &basisvector,
                  bool copy_children = false);
  shared_ptr<BasisVector> readBasisVector(const H5::H5Location &loc,
                                          const string &entry);

private:
  friend class DiscreteField;
  void noinsert(const shared_ptr<DiscreteField> &discretefield) {}
};
} // namespace SimulationIO

#define BASIS_HPP_DONE
#endif // #ifndef BASIS_HPP
#ifndef BASIS_HPP_DONE
#error "Cyclic include depencency"
#endif
