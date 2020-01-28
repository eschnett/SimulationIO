#ifndef DISCRETEFIELD_HPP
#define DISCRETEFIELD_HPP

#include "Basis.hpp"
#include "Common.hpp"
#include "Config.hpp"
#include "Configuration.hpp"
#include "Discretization.hpp"
#include "Field.hpp"

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
#include <asdf.hpp>
#endif

#ifdef SIMULATIONIO_HAVE_HDF5
#include <H5Cpp.h>
#endif

#ifdef SIMULATIONIO_HAVE_SILO
#include <silo.h>
#endif

#ifdef SIMULATIONIO_HAVE_TILEDB
#include <tiledb/tiledb>
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

class DiscreteFieldBlock;

class DiscreteField : public Common,
                      public std::enable_shared_from_this<DiscreteField> {
  weak_ptr<Field> m_field;                     // parent
  shared_ptr<Configuration> m_configuration;   // with backlink
  shared_ptr<Discretization> m_discretization; // with backlink
  shared_ptr<Basis> m_basis;                   // with backlink
  map<string, shared_ptr<DiscreteFieldBlock>> m_discretefieldblocks; // children
public:
  virtual string type() const { return "DiscreteField"; }

  shared_ptr<Field> field() const { return m_field.lock(); }
  shared_ptr<Configuration> configuration() const { return m_configuration; }
  shared_ptr<Discretization> discretization() const { return m_discretization; }
  shared_ptr<Basis> basis() const { return m_basis; }
  const map<string, shared_ptr<DiscreteFieldBlock>> &
  discretefieldblocks() const {
    return m_discretefieldblocks;
  }
  // TODO: Introduce map from DiscretizationBlock to DiscreteFieldBlock

  virtual bool invariant() const;

  DiscreteField() = delete;
  DiscreteField(const DiscreteField &) = delete;
  DiscreteField(DiscreteField &&) = delete;
  DiscreteField &operator=(const DiscreteField &) = delete;
  DiscreteField &operator=(DiscreteField &&) = delete;

  friend class Field;
  DiscreteField(hidden, const string &name, const shared_ptr<Field> &field,
                const shared_ptr<Configuration> &configuration,
                const shared_ptr<Discretization> &discretization,
                const shared_ptr<Basis> &basis)
      : Common(name), m_field(field), m_configuration(configuration),
        m_discretization(discretization), m_basis(basis) {}
  DiscreteField(hidden) : Common(hidden()) {}

private:
  static shared_ptr<DiscreteField>
  create(const string &name, const shared_ptr<Field> &field,
         const shared_ptr<Configuration> &configuration,
         const shared_ptr<Discretization> &discretization,
         const shared_ptr<Basis> &basis) {
    auto discretefield = make_shared<DiscreteField>(
        hidden(), name, field, configuration, discretization, basis);
    configuration->insert(name, discretefield);
    return discretefield;
  }
#ifdef SIMULATIONIO_HAVE_HDF5
  static shared_ptr<DiscreteField> create(const H5::H5Location &loc,
                                          const string &entry,
                                          const shared_ptr<Field> &field) {
    auto discretefield = make_shared<DiscreteField>(hidden());
    discretefield->read(loc, entry, field);
    return discretefield;
  }
  void read(const H5::H5Location &loc, const string &entry,
            const shared_ptr<Field> &field);
#endif
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
  static shared_ptr<DiscreteField>
  create(const shared_ptr<ASDF::reader_state> &rs, const YAML::Node &node,
         const shared_ptr<Field> &field) {
    auto discretefield = make_shared<DiscreteField>(hidden());
    discretefield->read(rs, node, field);
    return discretefield;
  }
  void read(const shared_ptr<ASDF::reader_state> &rs, const YAML::Node &node,
            const shared_ptr<Field> &field);
#endif

public:
  virtual ~DiscreteField() {}

  void merge(const shared_ptr<DiscreteField> &discretefield);

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os, const DiscreteField &discretefield) {
    return discretefield.output(os);
  }
#ifdef SIMULATIONIO_HAVE_HDF5
  virtual void write(const H5::H5Location &loc,
                     const H5::H5Location &parent) const;
#endif
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
  virtual vector<string> yaml_path() const;
  ASDF::writer &write(ASDF::writer &w) const;
  friend ASDF::writer &operator<<(ASDF::writer &w,
                                  const DiscreteField &discretefield) {
    return discretefield.write(w);
  }
#endif
#ifdef SIMULATIONIO_HAVE_SILO
  virtual string silo_path() const;
  virtual void write(DBfile *file, const string &loc) const;
#endif
#ifdef SIMULATIONIO_HAVE_TILEDB
  virtual vector<string> tiledb_path() const;
  virtual void write(const tiledb::Context &ctx, const string &loc) const;
#endif

  shared_ptr<DiscreteFieldBlock> createDiscreteFieldBlock(
      const string &name,
      const shared_ptr<DiscretizationBlock> &discretizationblock);
  shared_ptr<DiscreteFieldBlock> getDiscreteFieldBlock(
      const string &name,
      const shared_ptr<DiscretizationBlock> &discretizationblock);
  shared_ptr<DiscreteFieldBlock> copyDiscreteFieldBlock(
      const shared_ptr<DiscreteFieldBlock> &discretefieldblock,
      bool copy_children = false);
#ifdef SIMULATIONIO_HAVE_HDF5
  shared_ptr<DiscreteFieldBlock>
  readDiscreteFieldBlock(const H5::H5Location &loc, const string &entry);
#endif
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
  shared_ptr<DiscreteFieldBlock>
  readDiscreteFieldBlock(const shared_ptr<ASDF::reader_state> &rs,
                         const YAML::Node &node);
#endif
};

} // namespace SimulationIO

#define DISCRETEFIELD_HPP_DONE
#endif // #ifndef DISCRETEFIELD_HPP
#ifndef DISCRETEFIELD_HPP_DONE
#error "Cyclic include depencency"
#endif
