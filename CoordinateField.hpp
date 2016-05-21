#ifndef COORDINATEFIELD_HPP
#define COORDINATEFIELD_HPP

#include "Common.hpp"
#include "CoordinateSystem.hpp"
#include "Field.hpp"

#include <H5Cpp.h>

#include <iostream>
#include <memory>
#include <string>

using std::make_shared;
using std::ostream;
using std::shared_ptr;
using std::string;
using std::weak_ptr;

namespace SimulationIO {

class CoordinateField : public Common,
                        public std::enable_shared_from_this<CoordinateField> {
public:
  weak_ptr<CoordinateSystem> coordinatesystem; // parent
  int direction;
  shared_ptr<Field> field; // no backlink

  virtual bool invariant() const {
    return Common::invariant() && bool(coordinatesystem.lock()) &&
           coordinatesystem.lock()->coordinatefields.count(name()) &&
           coordinatesystem.lock()->coordinatefields.at(name()).get() == this &&
           direction >= 0 &&
           direction < coordinatesystem.lock()->manifold->dimension() &&
           coordinatesystem.lock()->directions.count(direction) &&
           coordinatesystem.lock()->directions.at(direction).get() == this &&
           bool(field) && field->coordinatefields.nobacklink();
    // TODO: Ensure that the field lives on the same manifold
    // TODO: Ensure that all fields of this coordinate system are distinct
    // TODO: Ensure the field is a scalar
  }

  CoordinateField() = delete;
  CoordinateField(const CoordinateField &) = delete;
  CoordinateField(CoordinateField &&) = delete;
  CoordinateField &operator=(const CoordinateField &) = delete;
  CoordinateField &operator=(CoordinateField &&) = delete;

  friend class CoordinateSystem;
  CoordinateField(hidden, const string &name,
                  const shared_ptr<CoordinateSystem> &coordinatesystem,
                  int direction, const shared_ptr<Field> &field)
      : Common(name), coordinatesystem(coordinatesystem), direction(direction),
        field(field) {}
  CoordinateField(hidden) : Common(hidden()) {}

private:
  static shared_ptr<CoordinateField>
  create(const string &name,
         const shared_ptr<CoordinateSystem> &coordinatesystem, int direction,
         const shared_ptr<Field> &field) {
    auto coordinatefield = make_shared<CoordinateField>(
        hidden(), name, coordinatesystem, direction, field);
    field->noinsert(coordinatefield);
    return coordinatefield;
  }
  static shared_ptr<CoordinateField>
  create(const H5::CommonFG &loc, const string &entry,
         const shared_ptr<CoordinateSystem> &coordinatesystem) {
    auto coordinatefield = make_shared<CoordinateField>(hidden());
    coordinatefield->read(loc, entry, coordinatesystem);
    return coordinatefield;
  }
  void read(const H5::CommonFG &loc, const string &entry,
            const shared_ptr<CoordinateSystem> &coordinatesystem);

public:
  virtual ~CoordinateField() {}

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os,
                             const CoordinateField &coordinatefield) {
    return coordinatefield.output(os);
  }
  virtual void write(const H5::CommonFG &loc,
                     const H5::H5Location &parent) const;
};
}

#define COORDINATEFIELD_HPP_DONE
#endif // #ifndef COORDINATEFIELD_HPP
#ifndef COORDINATEFIELD_HPP_DONE
#error "Cyclic include depencency"
#endif
