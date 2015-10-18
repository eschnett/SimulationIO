#ifndef DISCRETEFIELDBLOCKDATA_HPP
#define DISCRETEFIELDBLOCKDATA_HPP

#include "Common.hpp"
#include "DiscreteFieldBlock.hpp"
#include "TensorComponent.hpp"

#include <H5Cpp.h>

#include <cassert>
#include <iostream>
#include <map>
#include <string>

namespace SimulationIO {

using std::ostream;
using std::map;
using std::string;

struct DiscreteFieldBlockData : Common {
  // Tensor component for a discrete field on a particular region
  DiscreteFieldBlock *discretefieldblock; // parent
  TensorComponent *tensorcomponent;       // without backlink
  bool have_extlink;
  string extlink_file_name, extlink_obj_name;

  virtual bool invariant() const {
    bool inv = Common::invariant() && bool(discretefieldblock) &&
               discretefieldblock->discretefieldblockdata.count(name) &&
               discretefieldblock->discretefieldblockdata.at(name) == this &&
               bool(tensorcomponent) &&
               tensorcomponent->discretefieldblockdata.nobacklink() &&
               discretefieldblock->discretefield->field->tensortype ==
                   tensorcomponent->tensortype;
    if (have_extlink)
      inv &= !extlink_file_name.empty() && !extlink_obj_name.empty();
    return inv;
  }

  DiscreteFieldBlockData() = delete;
  DiscreteFieldBlockData(const DiscreteFieldBlockData &) = delete;
  DiscreteFieldBlockData(DiscreteFieldBlockData &&) = delete;
  DiscreteFieldBlockData &operator=(const DiscreteFieldBlockData &) = delete;
  DiscreteFieldBlockData &operator=(DiscreteFieldBlockData &&) = delete;

private:
  friend class DiscreteFieldBlock;
  DiscreteFieldBlockData(const string &name,
                         DiscreteFieldBlock *discretefieldblock,
                         TensorComponent *tensorcomponent)
      : Common(name), discretefieldblock(discretefieldblock),
        tensorcomponent(tensorcomponent), have_extlink(false) {
    tensorcomponent->noinsert(this);
  }
  DiscreteFieldBlockData(const H5::CommonFG &loc, const string &entry,
                         DiscreteFieldBlock *discretefieldblock);

public:
  virtual ~DiscreteFieldBlockData() { assert(0); }

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
