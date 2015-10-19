#ifndef DISCRETEFIELDBLOCKDATA_HPP
#define DISCRETEFIELDBLOCKDATA_HPP

#include "Common.hpp"
#include "DiscreteFieldBlock.hpp"
#include "TensorComponent.hpp"

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

struct DiscreteFieldBlockData
    : Common,
      std::enable_shared_from_this<DiscreteFieldBlockData> {
  // Tensor component for a discrete field on a particular region
  weak_ptr<DiscreteFieldBlock> discretefieldblock; // parent
  shared_ptr<TensorComponent> tensorcomponent;     // without backlink
  bool have_extlink;
  string extlink_file_name, extlink_obj_name;

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
    if (have_extlink)
      inv &= !extlink_file_name.empty() && !extlink_obj_name.empty();
    return inv;
  }

  DiscreteFieldBlockData() = delete;
  DiscreteFieldBlockData(const DiscreteFieldBlockData &) = delete;
  DiscreteFieldBlockData(DiscreteFieldBlockData &&) = delete;
  DiscreteFieldBlockData &operator=(const DiscreteFieldBlockData &) = delete;
  DiscreteFieldBlockData &operator=(DiscreteFieldBlockData &&) = delete;

  friend class DiscreteFieldBlock;
  DiscreteFieldBlockData(
      hidden, const string &name,
      const shared_ptr<DiscreteFieldBlock> &discretefieldblock,
      const shared_ptr<TensorComponent> &tensorcomponent)
      : Common(name), discretefieldblock(discretefieldblock),
        tensorcomponent(tensorcomponent), have_extlink(false) {}
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

  void setExternalLink(const string &file_name, const string &obj_name);

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
