#ifndef DISCRETEFIELD_HPP
#define DISCRETEFIELD_HPP

#include "Basis.hpp"
#include "Common.hpp"
#include "Discretization.hpp"
#include "Field.hpp"

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

struct DiscreteFieldBlock;

struct DiscreteField : Common, std::enable_shared_from_this<DiscreteField> {
  shared_ptr<Field> field;                   // parent
  shared_ptr<Discretization> discretization; // with backlink
  shared_ptr<Basis> basis;                   // with backlink
  map<string, shared_ptr<DiscreteFieldBlock>> discretefieldblocks; // children

  virtual bool invariant() const {
    return Common::invariant() && bool(field) &&
           field->discretefields.count(name) &&
           field->discretefields.at(name).get() == this &&
           bool(discretization) &&
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

  friend class Field;
  DiscreteField(hidden, const string &name, const shared_ptr<Field> &field,
                const shared_ptr<Discretization> &discretization,
                const shared_ptr<Basis> &basis)
      : Common(name), field(field), discretization(discretization),
        basis(basis) {}
  DiscreteField(hidden) : Common(hidden()) {}

private:
  static shared_ptr<DiscreteField>
  create(const string &name, const shared_ptr<Field> &field,
         const shared_ptr<Discretization> &discretization,
         const shared_ptr<Basis> &basis) {
    return make_shared<DiscreteField>(hidden(), name, field, discretization,
                                      basis);
  }
  static shared_ptr<DiscreteField> create(const H5::CommonFG &loc,
                                          const string &entry,
                                          const shared_ptr<Field> &field) {
    auto discretefield = make_shared<DiscreteField>(hidden());
    discretefield->read(loc, entry, field);
    return discretefield;
  }
  void read(const H5::CommonFG &loc, const string &entry,
            const shared_ptr<Field> &field);

public:
  virtual ~DiscreteField() {}

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os, const DiscreteField &discretefield) {
    return discretefield.output(os);
  }
  virtual void write(const H5::CommonFG &loc,
                     const H5::H5Location &parent) const;

  shared_ptr<DiscreteFieldBlock> createDiscreteFieldBlock(
      const string &name,
      const shared_ptr<DiscretizationBlock> &discretizationblock);
  shared_ptr<DiscreteFieldBlock>
  createDiscreteFieldBlock(const H5::CommonFG &loc, const string &entry);
};
}

#define DISCRETEFIELD_HPP_DONE
#endif // #ifndef DISCRETEFIELD_HPP
#ifndef DISCRETEFIELD_HPP_DONE
#error "Cyclic include depencency"
#endif
