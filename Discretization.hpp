#ifndef DISCRETIZATION_HPP
#define DISCRETIZATION_HPP

#include "Common.hpp"
#include "Configuration.hpp"
#include "Manifold.hpp"

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

class DiscreteField;
class DiscretizationBlock;
class SubDiscretization;

class Discretization : public Common,
                       public std::enable_shared_from_this<Discretization> {
  weak_ptr<Manifold> m_manifold;             // parent
  shared_ptr<Configuration> m_configuration; // with backlink
  map<string, shared_ptr<DiscretizationBlock>>
      m_discretizationblocks;                                       // children
  map<string, weak_ptr<SubDiscretization>> m_child_discretizations; // backlinks
  map<string, weak_ptr<SubDiscretization>>
      m_parent_discretizations; // backlinks
  NoBackLink<weak_ptr<DiscreteField>> m_discretefields;

public:
  shared_ptr<Manifold> manifold() const { return m_manifold.lock(); }
  shared_ptr<Configuration> configuration() const { return m_configuration; }
  const map<string, shared_ptr<DiscretizationBlock>> &
  discretizationblocks() const {
    return m_discretizationblocks;
  }
  const map<string, weak_ptr<SubDiscretization>> &
  child_discretizations() const {
    return m_child_discretizations;
  }
  const map<string, weak_ptr<SubDiscretization>> &
  parent_discretizations() const {
    return m_parent_discretizations;
  }
  NoBackLink<weak_ptr<DiscreteField>> discretefields() const {
    return m_discretefields;
  }

  virtual bool invariant() const;

  Discretization() = delete;
  Discretization(const Discretization &) = delete;
  Discretization(Discretization &&) = delete;
  Discretization &operator=(const Discretization &) = delete;
  Discretization &operator=(Discretization &&) = delete;

  friend class Manifold;
  Discretization(hidden, const string &name,
                 const shared_ptr<Manifold> &manifold,
                 const shared_ptr<Configuration> &configuration)
      : Common(name), m_manifold(manifold), m_configuration(configuration) {}
  Discretization(hidden) : Common(hidden()) {}

private:
  static shared_ptr<Discretization>
  create(const string &name, const shared_ptr<Manifold> &manifold,
         const shared_ptr<Configuration> &configuration) {
    auto discretization =
        make_shared<Discretization>(hidden(), name, manifold, configuration);
    configuration->insert(name, discretization);
    return discretization;
  }
  static shared_ptr<Discretization>
  create(const H5::H5Location &loc, const string &entry,
         const shared_ptr<Manifold> &manifold) {
    auto discretization = make_shared<Discretization>(hidden());
    discretization->read(loc, entry, manifold);
    return discretization;
  }
  void read(const H5::H5Location &loc, const string &entry,
            const shared_ptr<Manifold> &manifold);

public:
  virtual ~Discretization() {}

  void merge(const shared_ptr<Discretization> &discretization);

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os,
                             const Discretization &discretization) {
    return discretization.output(os);
  }
  virtual void write(const H5::H5Location &loc,
                     const H5::H5Location &parent) const;

  shared_ptr<DiscretizationBlock> createDiscretizationBlock(const string &name);
  shared_ptr<DiscretizationBlock> getDiscretizationBlock(const string &name);
  shared_ptr<DiscretizationBlock> copyDiscretizationBlock(
      const shared_ptr<DiscretizationBlock> &discretizationblock,
      bool copy_children = false);
  shared_ptr<DiscretizationBlock>
  readDiscretizationBlock(const H5::H5Location &loc, const string &entry);

private:
  friend class SubDiscretization;
  void insertChild(const string &name,
                   const shared_ptr<SubDiscretization> &subdiscretization) {
    checked_emplace(m_child_discretizations, name, subdiscretization,
                    "Discretization", "child_discretizations");
  }
  void insertParent(const string &name,
                    const shared_ptr<SubDiscretization> &subdiscretization) {
    checked_emplace(m_parent_discretizations, name, subdiscretization,
                    "Discretization", "parent_discretizations");
  }
  friend class DiscreteField;
  void noinsert(const shared_ptr<DiscreteField> &discretefield) {}
};
} // namespace SimulationIO

#define DISCRETIZATION_HPP_DONE
#endif // #ifndef DISCRETIZATION_HPP
#ifndef DISCRETIZATION_HPP_DONE
#error "Cyclic include depencency"
#endif
