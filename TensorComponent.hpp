#ifndef TENSORCOMPONENT_HPP
#define TENSORCOMPONENT_HPP

#include "Common.hpp"
#include "TensorType.hpp"

#include <H5Cpp.h>

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

namespace SimulationIO {

using std::iostream;
using std::string;
using std::vector;

struct DiscreteFieldBlockData;

struct TensorComponent : Common {
  TensorType *tensortype; // parent
  int storage_index;
  vector<int> indexvalues;
  NoBackLink<DiscreteFieldBlockData *> discretefieldblockdata;

  virtual bool invariant() const {
    bool inv = Common::invariant() && bool(tensortype) &&
               tensortype->tensorcomponents[name] == this &&
               storage_index >= 0 &&
               storage_index < ipow(tensortype->dimension, tensortype->rank) &&
               tensortype->storage_indices.count(storage_index) &&
               tensortype->storage_indices.at(storage_index) == this &&
               int(indexvalues.size()) == tensortype->rank;
    for (int i = 0; i < int(indexvalues.size()); ++i)
      inv &= indexvalues[i] >= 0 && indexvalues[i] < tensortype->dimension;
    // Ensure all tensor components are distinct
    for (const auto &tc : tensortype->tensorcomponents) {
      const auto &other = tc.second;
      if (other == this)
        continue;
      bool samesize = other->indexvalues.size() == indexvalues.size();
      inv &= samesize;
      if (samesize) {
        bool isequal = true;
        for (int i = 0; i < int(indexvalues.size()); ++i)
          isequal &= other->indexvalues[i] == indexvalues[i];
        inv &= !isequal;
      }
    }
    return inv;
  }

  TensorComponent() = delete;
  TensorComponent(const TensorComponent &) = delete;
  TensorComponent(TensorComponent &&) = delete;
  TensorComponent &operator=(const TensorComponent &) = delete;
  TensorComponent &operator=(TensorComponent &&) = delete;

private:
  friend class TensorType;
  TensorComponent(const string &name, TensorType *tensortype, int storage_index,
                  const vector<int> &indexvalues)
      : Common(name), tensortype(tensortype), storage_index(storage_index),
        indexvalues(indexvalues) {}
  TensorComponent(const H5::CommonFG &loc, const string &entry,
                  TensorType *tensortype);

public:
  virtual ~TensorComponent() { assert(0); }

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os,
                             const TensorComponent &tensorcomponent) {
    return tensorcomponent.output(os);
  }
  virtual void write(const H5::CommonFG &loc,
                     const H5::H5Location &parent) const;

  void noinsert(DiscreteFieldBlockData *discretefieldblockdata) {}
};
}

#define TENSORCOMPONENT_HPP_DONE
#endif // #ifndef TENSORCOMPONENT_HPP
#ifndef TENSORCOMPONENT_HPP_DONE
#error "Cyclic include depencency"
#endif
