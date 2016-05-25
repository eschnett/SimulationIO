#ifndef DISCRETEFIELDBLOCKCOMPONENT_HPP
#define DISCRETEFIELDBLOCKCOMPONENT_HPP

#include "Common.hpp"
#include "DiscreteFieldBlock.hpp"
#include "TensorComponent.hpp"

#include "H5Helpers.hpp"

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
using std::vector;
using std::weak_ptr;

class DiscreteFieldBlockComponent
    : public Common,
      public std::enable_shared_from_this<DiscreteFieldBlockComponent> {
  // Tensor component for a discrete field on a particular region
  weak_ptr<DiscreteFieldBlock> m_discretefieldblock; // parent
  shared_ptr<TensorComponent> m_tensorcomponent;     // without backlink
public:
  shared_ptr<DiscreteFieldBlock> discretefieldblock() const {
    return m_discretefieldblock.lock();
  }
  shared_ptr<TensorComponent> tensorcomponent() const {
    return m_tensorcomponent;
  }

  enum {
    type_empty,
    type_dataset,
    type_extlink,
    type_copy,
    type_range
  } data_type;
  H5::DataSpace data_dataspace;
  H5::DataType data_datatype;
  mutable H5::DataSet data_dataset;
  string data_extlink_filename, data_extlink_objname;
  H5::hid data_copy_loc;
  string data_copy_name;
  double data_range_origin;
  vector<double> data_range_delta;

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
                   .get() == this &&
           (data_type == type_empty || data_type == type_dataset ||
            data_type == type_extlink || data_type == type_copy ||
            data_type == type_range) &&
           !data_extlink_filename.empty() == (data_type == type_extlink) &&
           !data_extlink_objname.empty() == (data_type == type_extlink) &&
           data_copy_loc.valid() == (data_type == type_copy) &&
           !data_copy_name.empty() == (data_type == type_copy) &&
           !data_range_delta.empty() == (data_type == type_range) &&
           (int(data_range_delta.size()) ==
            discretefieldblock()
                ->discretizationblock()
                ->discretization()
                ->manifold()
                ->dimension()) == (data_type == type_range);
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
        m_tensorcomponent(tensorcomponent), data_type(type_empty) {}
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

  void setData();
  template <typename T> void setData();
  void setData(const string &filename, const string &objname);
  void setData(const H5::H5Location &loc, const string &name);
  void setData(double origin, const vector<double> &delta);

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &
  operator<<(ostream &os,
             const DiscreteFieldBlockComponent &discretefieldblockcomponent) {
    return discretefieldblockcomponent.output(os);
  }
  virtual void write(const H5::CommonFG &loc,
                     const H5::H5Location &parent) const;

  string getPath() const;
  string getName() const;
  // This expects that setData was called to create a dataset
  template <typename T> void writeData(const vector<T> &data) const;
};
}

#define DISCRETEFIELDBLOCKCOMPONENT_HPP_DONE
#endif // #ifndef DISCRETEFIELDBLOCKCOMPONENT_HPP
#ifndef DISCRETEFIELDBLOCKCOMPONENT_HPP_DONE
#error "Cyclic include depencency"
#endif
