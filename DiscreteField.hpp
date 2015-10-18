#ifndef DISCRETEFIELD_HPP
#define DISCRETEFIELD_HPP

#include "Basis.hpp"
#include "Common.hpp"
#include "Discretization.hpp"
#include "Field.hpp"

#include <H5Cpp.h>

#include <cassert>
#include <iostream>
#include <map>
#include <string>

namespace SimulationIO {

using std::ostream;
using std::map;
using std::string;

struct DiscreteFieldBlock;

struct DiscreteField : Common {
  Field *field;                                          // parent
  Discretization *discretization;                        // with backlink
  Basis *basis;                                          // with backlink
  map<string, DiscreteFieldBlock *> discretefieldblocks; // children

  virtual bool invariant() const {
    return Common::invariant() && bool(field) &&
           field->discretefields.count(name) &&
           field->discretefields.at(name) == this && bool(discretization) &&
           discretization->discretefields.nobacklink() &&
           field->manifold == discretization->manifold && bool(basis) &&
           basis->discretefields.nobacklink() &&
           field->tangentspace == basis->tangentspace;
  }

  DiscreteField() = delete;
  DiscreteField(const DiscreteField &) = delete;
  DiscreteField(DiscreteField &&) = delete;
  DiscreteField &operator=(const DiscreteField &) = delete;
  DiscreteField &operator=(DiscreteField &&) = delete;

private:
  friend class Field;
  DiscreteField(const string &name, Field *field,
                Discretization *discretization, Basis *basis)
      : Common(name), field(field), discretization(discretization),
        basis(basis) {}
  DiscreteField(const H5::CommonFG &loc, const string &entry, Field *field);

public:
  virtual ~DiscreteField() { assert(0); }

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os, const DiscreteField &discretefield) {
    return discretefield.output(os);
  }
  virtual void write(const H5::CommonFG &loc,
                     const H5::H5Location &parent) const;

  DiscreteFieldBlock *
  createDiscreteFieldBlock(const string &name,
                           DiscretizationBlock *discretizationblock);
  DiscreteFieldBlock *createDiscreteFieldBlock(const H5::CommonFG &loc,
                                               const string &entry);
};
}

#define DISCRETEFIELD_HPP_DONE
#endif // #ifndef DISCRETEFIELD_HPP
#ifndef DISCRETEFIELD_HPP_DONE
#error "Cyclic include depencency"
#endif
