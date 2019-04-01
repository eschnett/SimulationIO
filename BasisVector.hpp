#ifndef BASISVECTOR_HPP
#define BASISVECTOR_HPP

#include "Basis.hpp"
#include "Common.hpp"
#include "Config.hpp"

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
#include <asdf.hpp>
#endif

#ifdef SIMULATIONIO_HAVE_HDF5
#include <H5Cpp.h>
#endif

#ifdef SIMULATIONIO_HAVE_TILEDB
#include <tiledb/tiledb>
#endif

#include <iostream>
#include <memory>
#include <string>

using std::make_shared;
using std::ostream;
using std::shared_ptr;
using std::string;
using std::weak_ptr;

namespace SimulationIO {

class BasisVector : public Common,
                    public std::enable_shared_from_this<BasisVector> {
  weak_ptr<Basis> m_basis; // parent
  int m_direction;

public:
  virtual string type() const { return "BasisVector"; }

  shared_ptr<Basis> basis() const { return m_basis.lock(); }
  int direction() const { return m_direction; }

  virtual bool invariant() const;

  BasisVector() = delete;
  BasisVector(const BasisVector &) = delete;
  BasisVector(BasisVector &&) = delete;
  BasisVector &operator=(const BasisVector &) = delete;
  BasisVector &operator=(BasisVector &&) = delete;

  friend class Basis;
  BasisVector(hidden, const string &name, const shared_ptr<Basis> &basis,
              int direction)
      : Common(name), m_basis(basis), m_direction(direction) {}
  BasisVector(hidden) : Common(hidden()) {}

private:
  static shared_ptr<BasisVector>
  create(const string &name, const shared_ptr<Basis> &basis, int direction) {
    return make_shared<BasisVector>(hidden(), name, basis, direction);
  }
#ifdef SIMULATIONIO_HAVE_HDF5
  static shared_ptr<BasisVector> create(const H5::H5Location &loc,
                                        const string &entry,
                                        const shared_ptr<Basis> &basis) {
    auto basisvector = make_shared<BasisVector>(hidden());
    basisvector->read(loc, entry, basis);
    return basisvector;
  }
  void read(const H5::H5Location &loc, const string &entry,
            const shared_ptr<Basis> &basis);
#endif
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
  static shared_ptr<BasisVector>
  create(const shared_ptr<ASDF::reader_state> &rs, const YAML::Node &node,
         const shared_ptr<Basis> &basis) {
    auto basisvector = make_shared<BasisVector>(hidden());
    basisvector->read(rs, node, basis);
    return basisvector;
  }
  void read(const shared_ptr<ASDF::reader_state> &rs, const YAML::Node &node,
            const shared_ptr<Basis> &basis);
#endif

public:
  virtual ~BasisVector() {}

  void merge(const shared_ptr<BasisVector> &basisvector);

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os, const BasisVector &basisvector) {
    return basisvector.output(os);
  }
#ifdef SIMULATIONIO_HAVE_HDF5
  virtual void write(const H5::H5Location &loc,
                     const H5::H5Location &parent) const;
#endif
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
  virtual vector<string> yaml_path() const;
  ASDF::writer &write(ASDF::writer &w) const;
  friend ASDF::writer &operator<<(ASDF::writer &w,
                                  const BasisVector &basisvector) {
    return basisvector.write(w);
  }
#endif
#ifdef SIMULATIONIO_HAVE_TILEDB
  virtual vector<string> tiledb_path() const;
  virtual void write(const tiledb::Context &ctx, const string &loc) const;
#endif
};

} // namespace SimulationIO

#define BASISVECTOR_HPP_DONE
#endif // #ifndef BASISVECTOR_HPP
#ifndef BASISVECTOR_HPP_DONE
#error "Cyclic include depencency"
#endif
