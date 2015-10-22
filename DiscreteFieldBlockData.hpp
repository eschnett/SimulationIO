#ifndef DISCRETEFIELDBLOCKDATA_HPP
#define DISCRETEFIELDBLOCKDATA_HPP

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

struct DiscreteFieldBlockData
    : Common,
      std::enable_shared_from_this<DiscreteFieldBlockData> {
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
        discretefieldblock.lock()->discretefieldblockdata.count(name) &&
        discretefieldblock.lock()->discretefieldblockdata.at(name).get() ==
            this &&
        bool(tensorcomponent) &&
        tensorcomponent->discretefieldblockdata.nobacklink() &&
        discretefieldblock.lock()
                ->discretefield.lock()
                ->field.lock()
                ->tensortype.get() == tensorcomponent->tensortype.lock().get();
    // Ensure all discrete field block data have different tensor components
    for (const auto &dfbd : discretefieldblock.lock()->discretefieldblockdata)
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

  DiscreteFieldBlockData() = delete;
  DiscreteFieldBlockData(const DiscreteFieldBlockData &) = delete;
  DiscreteFieldBlockData(DiscreteFieldBlockData &&) = delete;
  DiscreteFieldBlockData &operator=(const DiscreteFieldBlockData &) = delete;
  DiscreteFieldBlockData &operator=(DiscreteFieldBlockData &&) = delete;

  friend struct DiscreteFieldBlock;
  DiscreteFieldBlockData(
      hidden, const string &name,
      const shared_ptr<DiscreteFieldBlock> &discretefieldblock,
      const shared_ptr<TensorComponent> &tensorcomponent)
      : Common(name), discretefieldblock(discretefieldblock),
        tensorcomponent(tensorcomponent), data_type(type_empty) {}
  DiscreteFieldBlockData(hidden) : Common(hidden()) {}

private:
  static shared_ptr<DiscreteFieldBlockData>
  create(const string &name,
         const shared_ptr<DiscreteFieldBlock> &discretefieldblock,
         const shared_ptr<TensorComponent> &tensorcomponent) {
    auto discretefieldblockdata = make_shared<DiscreteFieldBlockData>(
        hidden(), name, discretefieldblock, tensorcomponent);
    tensorcomponent->noinsert(discretefieldblockdata);
    return discretefieldblockdata;
  }
  static shared_ptr<DiscreteFieldBlockData>
  create(const H5::CommonFG &loc, const string &entry,
         const shared_ptr<DiscreteFieldBlock> &discretefieldblock) {
    auto discretefieldblockdata = make_shared<DiscreteFieldBlockData>(hidden());
    discretefieldblockdata->read(loc, entry, discretefieldblock);
    return discretefieldblockdata;
  }
  void read(const H5::CommonFG &loc, const string &entry,
            const shared_ptr<DiscreteFieldBlock> &discretefieldblock);

public:
  virtual ~DiscreteFieldBlockData() {}

  string getPath() const;
  string getName() const;
  void setData();
  void setData(const H5::DataType &datatype, const H5::DataSpace &dataspace);
  void setData(const string &filename, const string &objname);
  void setData(const H5::H5Location &loc, const string &name);

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &
  operator<<(ostream &os,
             const DiscreteFieldBlockData &discretefieldblockdata) {
    return discretefieldblockdata.output(os);
  }
  virtual void write(const H5::CommonFG &loc,
                     const H5::H5Location &parent) const;
};
}

#define DISCRETEFIELDBLOCKDATA_HPP_DONE
#endif // #ifndef DISCRETEFIELDBLOCKDATA_HPP
#ifndef DISCRETEFIELDBLOCKDATA_HPP_DONE
#error "Cyclic include depencency"
#endif
