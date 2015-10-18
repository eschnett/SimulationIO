#ifndef DISCRETEFIELDBLOCK_HPP
#define DISCRETEFIELDBLOCK_HPP

#include "Common.hpp"
#include "DiscreteField.hpp"
#include "DiscretizationBlock.hpp"

#include <H5Cpp.h>

#include <cassert>
#include <iostream>
#include <map>
#include <string>

namespace SimulationIO {

using std::ostream;
using std::map;
using std::string;

struct DiscreteFieldBlockData;

struct DiscreteFieldBlock : Common {
  // Discrete field on a particular region (discretization block)
  DiscreteField *discretefield;                                 // parent
  DiscretizationBlock *discretizationblock;                     // with backlink
  map<string, DiscreteFieldBlockData *> discretefieldblockdata; // children

  virtual bool invariant() const {
    return Common::invariant() && bool(discretefield) &&
           discretefield->discretefieldblocks.count(name) &&
           discretefield->discretefieldblocks.at(name) == this &&
           bool(discretizationblock) &&
           discretizationblock->discretefieldblocks.nobacklink();
  }

  DiscreteFieldBlock() = delete;
  DiscreteFieldBlock(const DiscreteFieldBlock &) = delete;
  DiscreteFieldBlock(DiscreteFieldBlock &&) = delete;
  DiscreteFieldBlock &operator=(const DiscreteFieldBlock &) = delete;
  DiscreteFieldBlock &operator=(DiscreteFieldBlock &&) = delete;

private:
  friend class DiscreteField;
  DiscreteFieldBlock(const string &name, DiscreteField *discretefield,
                     DiscretizationBlock *discretizationblock)
      : Common(name), discretefield(discretefield),
        discretizationblock(discretizationblock) {}
  DiscreteFieldBlock(const H5::CommonFG &loc, const string &entry,
                     DiscreteField *discretefield);

public:
  virtual ~DiscreteFieldBlock() { assert(0); }

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os,
                             const DiscreteFieldBlock &discretefieldblock) {
    return discretefieldblock.output(os);
  }
  virtual void write(const H5::CommonFG &loc,
                     const H5::H5Location &parent) const;

  DiscreteFieldBlockData *
  createDiscreteFieldBlockData(const string &name,
                               TensorComponent *tensorcomponent);
  DiscreteFieldBlockData *createDiscreteFieldBlockData(const H5::CommonFG &loc,
                                                       const string &entry);
};
}

#define DISCRETEFIELDBLOCK_HPP_DONE
#endif // #ifndef DISCRETEFIELDBLOCK_HPP
#ifndef DISCRETEFIELDBLOCK_HPP_DONE
#error "Cyclic include depencency"
#endif
