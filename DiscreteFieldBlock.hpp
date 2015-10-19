#ifndef DISCRETEFIELDBLOCK_HPP
#define DISCRETEFIELDBLOCK_HPP

#include "Common.hpp"
#include "DiscreteField.hpp"
#include "DiscretizationBlock.hpp"

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

struct DiscreteFieldBlockData;

struct DiscreteFieldBlock : Common,
                            std::enable_shared_from_this<DiscreteFieldBlock> {
  // Discrete field on a particular region (discretization block)
  shared_ptr<DiscreteField> discretefield;             // parent
  shared_ptr<DiscretizationBlock> discretizationblock; // with backlink
  map<string, shared_ptr<DiscreteFieldBlockData>>
      discretefieldblockdata; // children

  virtual bool invariant() const {
    return Common::invariant() && bool(discretefield) &&
           discretefield->discretefieldblocks.count(name) &&
           discretefield->discretefieldblocks.at(name).get() == this &&
           bool(discretizationblock) &&
           discretizationblock->discretefieldblocks.nobacklink();
  }

  DiscreteFieldBlock() = delete;
  DiscreteFieldBlock(const DiscreteFieldBlock &) = delete;
  DiscreteFieldBlock(DiscreteFieldBlock &&) = delete;
  DiscreteFieldBlock &operator=(const DiscreteFieldBlock &) = delete;
  DiscreteFieldBlock &operator=(DiscreteFieldBlock &&) = delete;

  friend class DiscreteField;
  DiscreteFieldBlock(hidden, const string &name,
                     const shared_ptr<DiscreteField> &discretefield,
                     const shared_ptr<DiscretizationBlock> &discretizationblock)
      : Common(name), discretefield(discretefield),
        discretizationblock(discretizationblock) {}
  DiscreteFieldBlock(hidden) : Common(hidden()) {}

private:
  static shared_ptr<DiscreteFieldBlock>
  create(const string &name, const shared_ptr<DiscreteField> &discretefield,
         const shared_ptr<DiscretizationBlock> &discretizationblock) {
    return make_shared<DiscreteFieldBlock>(hidden(), name, discretefield,
                                           discretizationblock);
  }
  static shared_ptr<DiscreteFieldBlock>
  create(const H5::CommonFG &loc, const string &entry,
         const shared_ptr<DiscreteField> &discretefield) {
    auto discretefieldblock = make_shared<DiscreteFieldBlock>(hidden());
    discretefieldblock->read(loc, entry, discretefield);
    return discretefieldblock;
  }
  void read(const H5::CommonFG &loc, const string &entry,
            const shared_ptr<DiscreteField> &discretefield);

public:
  virtual ~DiscreteFieldBlock() {}

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os,
                             const DiscreteFieldBlock &discretefieldblock) {
    return discretefieldblock.output(os);
  }
  virtual void write(const H5::CommonFG &loc,
                     const H5::H5Location &parent) const;

  shared_ptr<DiscreteFieldBlockData> createDiscreteFieldBlockData(
      const string &name, const shared_ptr<TensorComponent> &tensorcomponent);
  shared_ptr<DiscreteFieldBlockData>
  createDiscreteFieldBlockData(const H5::CommonFG &loc, const string &entry);
};
}

#define DISCRETEFIELDBLOCK_HPP_DONE
#endif // #ifndef DISCRETEFIELDBLOCK_HPP
#ifndef DISCRETEFIELDBLOCK_HPP_DONE
#error "Cyclic include depencency"
#endif
