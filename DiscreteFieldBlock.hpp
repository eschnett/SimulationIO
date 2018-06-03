#ifndef DISCRETEFIELDBLOCK_HPP
#define DISCRETEFIELDBLOCK_HPP

#include "Common.hpp"
#include "Config.hpp"
#include "DiscreteField.hpp"
#include "DiscretizationBlock.hpp"

#ifdef SIMULATIONIO_HAVE_ASDF_CXX
#include <asdf.hpp>
#endif

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

class DiscreteFieldBlockComponent;

class DiscreteFieldBlock
    : public Common,
      public std::enable_shared_from_this<DiscreteFieldBlock> {
  // Discrete field on a particular region (discretization block)
  weak_ptr<DiscreteField> m_discretefield;               // parent
  shared_ptr<DiscretizationBlock> m_discretizationblock; // with backlink
  map<string, shared_ptr<DiscreteFieldBlockComponent>>
      m_discretefieldblockcomponents; // children
  map<int, shared_ptr<DiscreteFieldBlockComponent>> m_storage_indices;

public:
  virtual string type() const { return "DiscreteFieldBlock"; }

  shared_ptr<DiscreteField> discretefield() const {
    return m_discretefield.lock();
  }
  shared_ptr<DiscretizationBlock> discretizationblock() const {
    return m_discretizationblock;
  }
  const map<string, shared_ptr<DiscreteFieldBlockComponent>> &
  discretefieldblockcomponents() const {
    return m_discretefieldblockcomponents;
  }
  const map<int, shared_ptr<DiscreteFieldBlockComponent>> &
  storage_indices() const {
    return m_storage_indices;
  }

  virtual bool invariant() const;

  DiscreteFieldBlock() = delete;
  DiscreteFieldBlock(const DiscreteFieldBlock &) = delete;
  DiscreteFieldBlock(DiscreteFieldBlock &&) = delete;
  DiscreteFieldBlock &operator=(const DiscreteFieldBlock &) = delete;
  DiscreteFieldBlock &operator=(DiscreteFieldBlock &&) = delete;

  friend class DiscreteField;
  DiscreteFieldBlock(hidden, const string &name,
                     const shared_ptr<DiscreteField> &discretefield,
                     const shared_ptr<DiscretizationBlock> &discretizationblock)
      : Common(name), m_discretefield(discretefield),
        m_discretizationblock(discretizationblock) {}
  DiscreteFieldBlock(hidden) : Common(hidden()) {}

private:
  static shared_ptr<DiscreteFieldBlock>
  create(const string &name, const shared_ptr<DiscreteField> &discretefield,
         const shared_ptr<DiscretizationBlock> &discretizationblock) {
    return make_shared<DiscreteFieldBlock>(hidden(), name, discretefield,
                                           discretizationblock);
  }
  static shared_ptr<DiscreteFieldBlock>
  create(const H5::H5Location &loc, const string &entry,
         const shared_ptr<DiscreteField> &discretefield) {
    auto discretefieldblock = make_shared<DiscreteFieldBlock>(hidden());
    discretefieldblock->read(loc, entry, discretefield);
    return discretefieldblock;
  }
  void read(const H5::H5Location &loc, const string &entry,
            const shared_ptr<DiscreteField> &discretefield);
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
  static shared_ptr<DiscreteFieldBlock>
  create(const ASDF::reader_state &rs, const YAML::Node &node,
         const shared_ptr<DiscreteField> &discretefield) {
    auto discretefieldblock = make_shared<DiscreteFieldBlock>(hidden());
    discretefieldblock->read(rs, node, discretefield);
    return discretefieldblock;
  }
  void read(const ASDF::reader_state &rs, const YAML::Node &node,
            const shared_ptr<DiscreteField> &discretefield);
#endif

public:
  virtual ~DiscreteFieldBlock() {}

  void merge(const shared_ptr<DiscreteFieldBlock> &discretefieldblock);

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os,
                             const DiscreteFieldBlock &discretefieldblock) {
    return discretefieldblock.output(os);
  }
  virtual void write(const H5::H5Location &loc,
                     const H5::H5Location &parent) const;
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
  virtual vector<string> yaml_path() const;
  ASDF::writer &write(ASDF::writer &w) const;
  friend ASDF::writer &
  operator<<(ASDF::writer &w, const DiscreteFieldBlock &discretefieldblock) {
    return discretefieldblock.write(w);
  }
#endif

  shared_ptr<DiscreteFieldBlockComponent> createDiscreteFieldBlockComponent(
      const string &name, const shared_ptr<TensorComponent> &tensorcomponent);
  shared_ptr<DiscreteFieldBlockComponent> getDiscreteFieldBlockComponent(
      const string &name, const shared_ptr<TensorComponent> &tensorcomponent);
  shared_ptr<DiscreteFieldBlockComponent>
  copyDiscreteFieldBlockComponent(const shared_ptr<DiscreteFieldBlockComponent>
                                      &discretefieldblockcomponent,
                                  bool copy_children = false);
  shared_ptr<DiscreteFieldBlockComponent>
  readDiscreteFieldBlockComponent(const H5::H5Location &loc,
                                  const string &entry);
#ifdef SIMULATIONIO_HAVE_ASDF_CXX
  shared_ptr<DiscreteFieldBlockComponent>
  readDiscreteFieldBlockComponent(const ASDF::reader_state &rs,
                                  const YAML::Node &node);
#endif
};

} // namespace SimulationIO

#define DISCRETEFIELDBLOCK_HPP_DONE
#endif // #ifndef DISCRETEFIELDBLOCK_HPP
#ifndef DISCRETEFIELDBLOCK_HPP_DONE
#error "Cyclic include depencency"
#endif
