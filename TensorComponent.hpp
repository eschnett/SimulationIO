#ifndef TENSORCOMPONENT_HPP
#define TENSORCOMPONENT_HPP

#include "Common.hpp"
#include "TensorType.hpp"

#include <H5Cpp.h>

#include <iostream>
#include <string>
#include <vector>

namespace SimulationIO {

using std::make_shared;
using std::ostream;
using std::shared_ptr;
using std::string;
using std::vector;
using std::weak_ptr;

class DiscreteFieldBlockComponent;

class TensorComponent : public Common,
                        public std::enable_shared_from_this<TensorComponent> {
  weak_ptr<TensorType> m_tensortype; // parent
  int m_storage_index;
  vector<int> m_indexvalues;
  NoBackLink<weak_ptr<DiscreteFieldBlockComponent>>
      m_discretefieldblockcomponents;

public:
  shared_ptr<TensorType> tensortype() const { return m_tensortype.lock(); }
  int storage_index() const { return m_storage_index; }
  vector<int> indexvalues() const { return m_indexvalues; }
  NoBackLink<weak_ptr<DiscreteFieldBlockComponent>>
  discretefieldblockcomponents() const {
    return m_discretefieldblockcomponents;
  }

  virtual bool invariant() const;

  TensorComponent() = delete;
  TensorComponent(const TensorComponent &) = delete;
  TensorComponent(TensorComponent &&) = delete;
  TensorComponent &operator=(const TensorComponent &) = delete;
  TensorComponent &operator=(TensorComponent &&) = delete;

  friend class TensorType;
  TensorComponent(hidden, const string &name,
                  const shared_ptr<TensorType> &tensortype, int storage_index,
                  const vector<int> &indexvalues)
      : Common(name), m_tensortype(tensortype), m_storage_index(storage_index),
        m_indexvalues(indexvalues) {}
  TensorComponent(hidden) : Common(hidden()) {}

private:
  static shared_ptr<TensorComponent>
  create(const string &name, const shared_ptr<TensorType> &tensortype,
         int storage_index, const vector<int> &indexvalues) {
    return make_shared<TensorComponent>(hidden(), name, tensortype,
                                        storage_index, indexvalues);
  }
  static shared_ptr<TensorComponent>
  create(const H5::H5Location &loc, const string &entry,
         const shared_ptr<TensorType> &tensortype) {
    auto tensorcomponent = make_shared<TensorComponent>(hidden());
    tensorcomponent->read(loc, entry, tensortype);
    return tensorcomponent;
  }
  void read(const H5::H5Location &loc, const string &entry,
            const shared_ptr<TensorType> &tensortype);

public:
  virtual ~TensorComponent() {}

  void merge(const shared_ptr<TensorComponent> &tensorcomponent);

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os,
                             const TensorComponent &tensorcomponent) {
    return tensorcomponent.output(os);
  }
  virtual void write(const H5::H5Location &loc,
                     const H5::H5Location &parent) const;

private:
  friend class DiscreteFieldBlockComponent;
  void noinsert(const shared_ptr<DiscreteFieldBlockComponent>
                    &discretefieldblockcomponent) {}
};
} // namespace SimulationIO

#define TENSORCOMPONENT_HPP_DONE
#endif // #ifndef TENSORCOMPONENT_HPP
#ifndef TENSORCOMPONENT_HPP_DONE
#error "Cyclic include depencency"
#endif
