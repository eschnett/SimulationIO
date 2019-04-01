#ifndef DISCRETIZATIONBLOCK_HPP
#define DISCRETIZATIONBLOCK_HPP

#include "Common.hpp"
#include "Config.hpp"
#include "Discretization.hpp"
#include "RegionCalculus.hpp"

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
#include <asdf.hpp>
#endif

#ifdef SIMULATIONIO_HAVE_HDF5
#include <H5Cpp.h>
#endif

#ifdef SIMULATIONIO_HAVE_TILEDB
#include <tiledb/tiledb>
#endif

#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace SimulationIO {

using namespace RegionCalculus;

using std::make_shared;
using std::ostream;
using std::shared_ptr;
using std::string;
using std::vector;
using std::weak_ptr;

class DiscreteFieldBlock;

class DiscretizationBlock
    : public Common,
      public std::enable_shared_from_this<DiscretizationBlock> {
  // Discretization of a certain region, represented by contiguous data
  weak_ptr<Discretization> m_discretization; // parent
  box_t m_box;
  region_t m_active;
  NoBackLink<weak_ptr<DiscreteFieldBlock>> m_discretefieldblocks;

public:
  virtual string type() const { return "DiscretizationBlock"; }

  shared_ptr<Discretization> discretization() const {
    return m_discretization.lock();
  }
  const box_t &box() const { return m_box; }
  const region_t &active() const { return m_active; }
  NoBackLink<weak_ptr<DiscreteFieldBlock>> discretefieldblocks() const {
    return m_discretefieldblocks;
  }

  // bounding box? in terms of coordinates?
  // connectivity? neighbouring blocks?
  // overlaps?

  virtual bool invariant() const;

  DiscretizationBlock() = delete;
  DiscretizationBlock(const DiscretizationBlock &) = delete;
  DiscretizationBlock(DiscretizationBlock &&) = delete;
  DiscretizationBlock &operator=(const DiscretizationBlock &) = delete;
  DiscretizationBlock &operator=(DiscretizationBlock &&) = delete;

  friend class Discretization;
  DiscretizationBlock(hidden, const string &name,
                      const shared_ptr<Discretization> &discretization)
      : Common(name), m_discretization(discretization) {}
  DiscretizationBlock(hidden) : Common(hidden()) {}

private:
  static shared_ptr<DiscretizationBlock>
  create(const string &name, const shared_ptr<Discretization> &discretization) {
    return make_shared<DiscretizationBlock>(hidden(), name, discretization);
  }
#ifdef SIMULATIONIO_HAVE_HDF5
  static shared_ptr<DiscretizationBlock>
  create(const H5::H5Location &loc, const string &entry,
         const shared_ptr<Discretization> &discretization) {
    auto discretizationblock = make_shared<DiscretizationBlock>(hidden());
    discretizationblock->read(loc, entry, discretization);
    return discretizationblock;
  }
  void read(const H5::H5Location &loc, const string &entry,
            const shared_ptr<Discretization> &discretization);
#endif
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
  static shared_ptr<DiscretizationBlock>
  create(const shared_ptr<ASDF::reader_state> &rs, const YAML::Node &node,
         const shared_ptr<Discretization> &discretization) {
    auto discretizationblock = make_shared<DiscretizationBlock>(hidden());
    discretizationblock->read(rs, node, discretization);
    return discretizationblock;
  }
  void read(const shared_ptr<ASDF::reader_state> &rs, const YAML::Node &node,
            const shared_ptr<Discretization> &discretization);
#endif

public:
  virtual ~DiscretizationBlock() {}

  void merge(const shared_ptr<DiscretizationBlock> &discretizationblock);

  void setBox() { m_box.reset(); }
  void setBox(const box_t &box) {
    assert(box.valid() &&
           box.rank() == discretization()->manifold()->dimension() &&
           !box.empty());
    m_box = box;
  }
  box_t getBox() const { return m_box; }

  void setActive() { m_active.reset(); }
  void setActive(const region_t &active) {
    assert(active.valid() &&
           active.rank() == discretization()->manifold()->dimension());
    m_active = active;
  }
  region_t getActive() const { return m_active; }

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os,
                             const DiscretizationBlock &discretizationblock) {
    return discretizationblock.output(os);
  }
#ifdef SIMULATIONIO_HAVE_HDF5
  virtual void write(const H5::H5Location &loc,
                     const H5::H5Location &parent) const;
#endif
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
  virtual vector<string> yaml_path() const;
  ASDF::writer &write(ASDF::writer &w) const;
  friend ASDF::writer &
  operator<<(ASDF::writer &w, const DiscretizationBlock &discretizationblock) {
    return discretizationblock.write(w);
  }
#endif
#ifdef SIMULATIONIO_HAVE_TILEDB
  virtual vector<string> tiledb_path() const;
  virtual void write(const tiledb::Context &ctx, const string &loc) const;
#endif

private:
  friend class DiscreteFieldBlock;
  void noinsert(const shared_ptr<DiscreteFieldBlock> &discretefieldblock) {}
};

} // namespace SimulationIO

#define DISCRETIZATIONBLOCK_HPP_DONE
#endif // #ifndef DISCRETIZATIONBLOCK_HPP
#ifndef DISCRETIZATIONBLOCK_HPP_DONE
#error "Cyclic include depencency"
#endif
