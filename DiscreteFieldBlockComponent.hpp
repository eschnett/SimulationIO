#ifndef DISCRETEFIELDBLOCKCOMPONENT_HPP
#define DISCRETEFIELDBLOCKCOMPONENT_HPP

#include "Common.hpp"
#include "DataBlock.hpp"
#include "DiscreteFieldBlock.hpp"
#include "TensorComponent.hpp"

#include "H5Helpers.hpp"

#include <iostream>
#include <map>
#include <memory>
#include <string>

namespace SimulationIO {

using std::dynamic_pointer_cast;
using std::make_shared;
using std::map;
using std::ostream;
using std::shared_ptr;
using std::string;
using std::vector;
using std::weak_ptr;

class DiscreteFieldBlockComponent
    : public Common,
      public std::enable_shared_from_this<DiscreteFieldBlockComponent> {
  // Tensor component for a discrete field on a particular region
  weak_ptr<DiscreteFieldBlock> m_discretefieldblock; // parent
  shared_ptr<TensorComponent> m_tensorcomponent;     // without backlink
  shared_ptr<DataBlock> m_datablock;

  static string dataname() { return "data"; }

public:
  shared_ptr<DiscreteFieldBlock> discretefieldblock() const {
    return m_discretefieldblock.lock();
  }
  shared_ptr<TensorComponent> tensorcomponent() const {
    return m_tensorcomponent;
  }
  shared_ptr<DataBlock> datablock() const { return m_datablock; }
  shared_ptr<DataRange> datarange() const {
    return dynamic_pointer_cast<DataRange>(m_datablock);
  }
  shared_ptr<DataSet> dataset() const {
    return dynamic_pointer_cast<DataSet>(m_datablock);
  }
  shared_ptr<CopyObj> copyobj() const {
    return dynamic_pointer_cast<CopyObj>(m_datablock);
  }
  shared_ptr<ExtLink> extlink() const {
    return dynamic_pointer_cast<ExtLink>(m_datablock);
  }

  virtual bool invariant() const {
    bool inv =
        Common::invariant() && bool(discretefieldblock()) &&
        discretefieldblock()->discretefieldblockcomponents().count(name()) &&
        discretefieldblock()->discretefieldblockcomponents().at(name()).get() ==
            this &&
        bool(tensorcomponent()) &&
        tensorcomponent()->discretefieldblockcomponents().nobacklink() &&
        discretefieldblock()->discretefield()->field()->tensortype().get() ==
            tensorcomponent()->tensortype().get();
    // Ensure all discrete field block data have different tensor components
    for (const auto &dfbd :
         discretefieldblock()->discretefieldblockcomponents())
      if (dfbd.second.get() != this)
        inv &= dfbd.second->tensorcomponent().get() != tensorcomponent().get();
    // Ensure mapping from storage_indices is correct
    inv &= discretefieldblock()
               ->storage_indices()
               .at(tensorcomponent()->storage_index())
               .get() == this;
    return inv;
  }

  DiscreteFieldBlockComponent() = delete;
  DiscreteFieldBlockComponent(const DiscreteFieldBlockComponent &) = delete;
  DiscreteFieldBlockComponent(DiscreteFieldBlockComponent &&) = delete;
  DiscreteFieldBlockComponent &
  operator=(const DiscreteFieldBlockComponent &) = delete;
  DiscreteFieldBlockComponent &
  operator=(DiscreteFieldBlockComponent &&) = delete;

  friend class DiscreteFieldBlock;
  DiscreteFieldBlockComponent(
      hidden, const string &name,
      const shared_ptr<DiscreteFieldBlock> &discretefieldblock,
      const shared_ptr<TensorComponent> &tensorcomponent)
      : Common(name), m_discretefieldblock(discretefieldblock),
        m_tensorcomponent(tensorcomponent) {}
  DiscreteFieldBlockComponent(hidden) : Common(hidden()) {}

private:
  static shared_ptr<DiscreteFieldBlockComponent>
  create(const string &name,
         const shared_ptr<DiscreteFieldBlock> &discretefieldblock,
         const shared_ptr<TensorComponent> &tensorcomponent) {
    auto discretefieldblockcomponent = make_shared<DiscreteFieldBlockComponent>(
        hidden(), name, discretefieldblock, tensorcomponent);
    tensorcomponent->noinsert(discretefieldblockcomponent);
    return discretefieldblockcomponent;
  }
  static shared_ptr<DiscreteFieldBlockComponent>
  create(const H5::CommonFG &loc, const string &entry,
         const shared_ptr<DiscreteFieldBlock> &discretefieldblock) {
    auto discretefieldblockcomponent =
        make_shared<DiscreteFieldBlockComponent>(hidden());
    discretefieldblockcomponent->read(loc, entry, discretefieldblock);
    return discretefieldblockcomponent;
  }
  void read(const H5::CommonFG &loc, const string &entry,
            const shared_ptr<DiscreteFieldBlock> &discretefieldblock);

public:
  virtual ~DiscreteFieldBlockComponent() {}

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &
  operator<<(ostream &os,
             const DiscreteFieldBlockComponent &discretefieldblockcomponent) {
    return discretefieldblockcomponent.output(os);
  }
  virtual void write(const H5::CommonFG &loc,
                     const H5::H5Location &parent) const;

  void unsetDataBlock();
  shared_ptr<DataRange> createDataRange(double origin,
                                        const vector<double> &delta);
  template <typename T> shared_ptr<DataSet> createDataSet() {
    assert(!m_datablock);
    auto res = make_shared<DataSet>(
        T(), discretefieldblock()->discretizationblock()->box());
    m_datablock = res;
    return res;
  }
  shared_ptr<CopyObj> createCopyObj(const H5::Group &group, const string &name);
  shared_ptr<CopyObj> createCopyObj(const H5::H5File &file, const string &name);
  shared_ptr<ExtLink> createExtLink(const string &filename,
                                    const string &objname);

  string getPath() const;
  string getName() const;
};
}

#define DISCRETEFIELDBLOCKCOMPONENT_HPP_DONE
#endif // #ifndef DISCRETEFIELDBLOCKCOMPONENT_HPP
#ifndef DISCRETEFIELDBLOCKCOMPONENT_HPP_DONE
#error "Cyclic include depencency"
#endif
