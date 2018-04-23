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
  weak_ptr<CoordinateSystem> m_coordinatesystem; // parent
  int m_direction;
  shared_ptr<Field> m_field; // no backlink
public:
  shared_ptr<CoordinateSystem> coordinatesystem() const {
    return m_coordinatesystem.lock();
  }
  int direction() const { return m_direction; }
  const shared_ptr<Field> &field() const { return m_field; }

  virtual bool invariant() const;

  CoordinateField() = delete;
  CoordinateField(const CoordinateField &) = delete;
  CoordinateField(CoordinateField &&) = delete;
  CoordinateField &operator=(const CoordinateField &) = delete;
  CoordinateField &operator=(CoordinateField &&) = delete;

  friend class CoordinateSystem;
  CoordinateField(hidden, const string &name,
                  const shared_ptr<CoordinateSystem> &coordinatesystem,
                  int direction, const shared_ptr<Field> &field)
      : Common(name), m_coordinatesystem(coordinatesystem),
        m_direction(direction), m_field(field) {}
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
  create(const H5::H5Location &loc, const string &entry,
         const shared_ptr<CoordinateSystem> &coordinatesystem) {
    auto coordinatefield = make_shared<CoordinateField>(hidden());
    coordinatefield->read(loc, entry, coordinatesystem);
    return coordinatefield;
  }
  void read(const H5::H5Location &loc, const string &entry,
            const shared_ptr<CoordinateSystem> &coordinatesystem);

public:
  virtual ~CoordinateField() {}

  void merge(const shared_ptr<CoordinateField> &coordinatefield);

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os,
                             const CoordinateField &coordinatefield) {
    return coordinatefield.output(os);
  }
  virtual void write(const H5::H5Location &loc,
                     const H5::H5Location &parent) const;
};
} // namespace SimulationIO

#define COORDINATEFIELD_HPP_DONE
#endif // #ifndef COORDINATEFIELD_HPP
#ifndef COORDINATEFIELD_HPP_DONE
#error "Cyclic include depencency"
#endif
