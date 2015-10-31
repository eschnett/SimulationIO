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
using std::weak_ptr;

struct DiscreteFieldBlockComponent
    : Common,
      std::enable_shared_from_this<DiscreteFieldBlockComponent> {
  // Tensor component for a discrete field on a particular region
  weak_ptr<DiscreteFieldBlock> discretefieldblock; // parent
  shared_ptr<TensorComponent> tensorcomponent;     // without backlink
  enum { type_empty, type_dataset, type_extlink, type_copy } data_type;
  H5::DataSpace data_dataspace;
  H5::DataType data_datatype;
  mutable H5::DataSet data_dataset;
  string data_extlink_filename, data_extlink_objname;
  H5::hid data_copy_loc;
  string data_copy_name;

  virtual bool invariant() const {
    bool inv =
        Common::invariant() && bool(discretefieldblock.lock()) &&
        discretefieldblock.lock()->discretefieldblockcomponents.count(name) &&
        discretefieldblock.lock()
                ->discretefieldblockcomponents.at(name)
                .get() == this &&
        bool(tensorcomponent) &&
        tensorcomponent->discretefieldblockcomponents.nobacklink() &&
        discretefieldblock.lock()
                ->discretefield.lock()
                ->field.lock()
                ->tensortype.get() == tensorcomponent->tensortype.lock().get();
    // Ensure all discrete field block data have different tensor components
    for (const auto &dfbd :
         discretefieldblock.lock()->discretefieldblockcomponents)
      if (dfbd.second.get() != this)
        inv &= dfbd.second->tensorcomponent.get() != tensorcomponent.get();
    inv &= (data_type == type_empty || data_type == type_dataset ||
            data_type == type_extlink || data_type == type_copy) &&
           !data_extlink_filename.empty() == (data_type == type_extlink) &&
           !data_extlink_objname.empty() == (data_type == type_extlink) &&
           data_copy_loc.valid() == (data_type == type_copy) &&
           !data_copy_name.empty() == (data_type == type_copy);
    return inv;
  }

  DiscreteFieldBlockComponent() = delete;
  DiscreteFieldBlockComponent(const DiscreteFieldBlockComponent &) = delete;
  DiscreteFieldBlockComponent(DiscreteFieldBlockComponent &&) = delete;
  DiscreteFieldBlockComponent &
  operator=(const DiscreteFieldBlockComponent &) = delete;
  DiscreteFieldBlockComponent &
  operator=(DiscreteFieldBlockComponent &&) = delete;

  friend struct DiscreteFieldBlock;
  DiscreteFieldBlockComponent(
      hidden, const string &name,
      const shared_ptr<DiscreteFieldBlock> &discretefieldblock,
      const shared_ptr<TensorComponent> &tensorcomponent)
      : Common(name), discretefieldblock(discretefieldblock),
        tensorcomponent(tensorcomponent), data_type(type_empty) {}
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

  string getPath() const;
  string getName() const;
  void setData();
  void setData(const H5::DataType &datatype, const H5::DataSpace &dataspace);
  void setData(const string &filename, const string &objname);
  void setData(const H5::H5Location &loc, const string &name);

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &
  operator<<(ostream &os,
             const DiscreteFieldBlockComponent &discretefieldblockcomponent) {
    return discretefieldblockcomponent.output(os);
  }
  virtual void write(const H5::CommonFG &loc,
                     const H5::H5Location &parent) const;
};
}

#define DISCRETEFIELDBLOCKCOMPONENT_HPP_DONE
#endif // #ifndef DISCRETEFIELDBLOCKCOMPONENT_HPP
#ifndef DISCRETEFIELDBLOCKCOMPONENT_HPP_DONE
#error "Cyclic include depencency"
#endif
