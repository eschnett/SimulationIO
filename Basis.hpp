#ifndef BASIS_HPP
#define BASIS_HPP

#include "Common.hpp"
#include "Config.hpp"
#include "Configuration.hpp"
#include "TangentSpace.hpp"

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
#include <asdf.hpp>
#endif

#ifdef SIMULATIONIO_HAVE_HDF5
#include <H5Cpp.h>
#endif

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
  NoBackLink<weak_ptr<DiscreteField>> m_discretefield;
  // map<string, CoordinateBasis *> m_coordinatebases;
public:
  virtual string type() const { return "Basis"; }

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
  NoBackLink<weak_ptr<DiscreteField>> discretefield() const {
    return m_discretefield;
  }

  virtual bool invariant() const;

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
#ifdef SIMULATIONIO_HAVE_HDF5
  static shared_ptr<Basis>
  create(const H5::H5Location &loc, const string &entry,
         const shared_ptr<TangentSpace> &tangentspace) {
    auto basis = make_shared<Basis>(hidden());
    basis->read(loc, entry, tangentspace);
    return basis;
  }
  void read(const H5::H5Location &loc, const string &entry,
            const shared_ptr<TangentSpace> &tangentspace);
#endif
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
  static shared_ptr<Basis>
  create(const ASDF::reader_state &rs, const YAML::Node &node,
         const shared_ptr<TangentSpace> &tangentspace) {
    auto basis = make_shared<Basis>(hidden());
    basis->read(rs, node, tangentspace);
    return basis;
  }
  void read(const ASDF::reader_state &rs, const YAML::Node &node,
            const shared_ptr<TangentSpace> &tangentspace);
#endif

public:
  virtual ~Basis() {}

  void merge(const shared_ptr<Basis> &basis);

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os, const Basis &basis) {
    return basis.output(os);
  }
#ifdef SIMULATIONIO_HAVE_HDF5
  virtual void write(const H5::H5Location &loc,
                     const H5::H5Location &parent) const;
#endif
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
  virtual vector<string> yaml_path() const;
  ASDF::writer &write(ASDF::writer &w) const;
  friend ASDF::writer &operator<<(ASDF::writer &w, const Basis &basis) {
    return basis.write(w);
  }
#endif

  shared_ptr<BasisVector> createBasisVector(const string &name, int direction);
  shared_ptr<BasisVector> getBasisVector(const string &name, int direction);
  shared_ptr<BasisVector>
  copyBasisVector(const shared_ptr<BasisVector> &basisvector,
                  bool copy_children = false);
#ifdef SIMULATIONIO_HAVE_HDF5
  shared_ptr<BasisVector> readBasisVector(const H5::H5Location &loc,
                                          const string &entry);
#endif
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
  shared_ptr<BasisVector> readBasisVector(const ASDF::reader_state &rs,
                                          const YAML::Node &node);
#endif

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
