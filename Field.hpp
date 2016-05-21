#ifndef FIELD_HPP
#define FIELD_HPP

#include "Common.hpp"
#include "Configuration.hpp"
#include "Manifold.hpp"
#include "Project.hpp"
#include "TangentSpace.hpp"
#include "TensorType.hpp"

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

class CoordinateField;
class Manifold;
class TangentSpace;
class TensorType;
class DiscreteField;

class Field : public Common, public std::enable_shared_from_this<Field> {
public:
  weak_ptr<Project> project;                             // parent
  shared_ptr<Configuration> configuration;               // with backlink
  shared_ptr<Manifold> manifold;                         // with backlink
  shared_ptr<TangentSpace> tangentspace;                 // with backlink
  shared_ptr<TensorType> tensortype;                     // without backlink
  map<string, shared_ptr<DiscreteField>> discretefields; // children
  NoBackLink<CoordinateField> coordinatefields;

  virtual bool invariant() const {
    bool inv = Common::invariant() && bool(project.lock()) &&
               project.lock()->fields.count(name()) &&
               project.lock()->fields.at(name()).get() == this &&
               bool(configuration) && configuration->fields().count(name()) &&
               configuration->fields().at(name()).lock().get() == this &&
               bool(manifold) && manifold->fields().count(name()) &&
               manifold->fields().at(name()).lock().get() == this &&
               bool(tangentspace) && tangentspace->fields().count(name()) &&
               tangentspace->fields().at(name()).lock().get() == this &&
               bool(tensortype) &&
               tangentspace->dimension == tensortype->dimension &&
               tensortype->fields.nobacklink();
    for (const auto &df : discretefields)
      inv &= !df.first.empty() && bool(df.second);
    return inv;
  }

  Field() = delete;
  Field(const Field &) = delete;
  Field(Field &&) = delete;
  Field &operator=(const Field &) = delete;
  Field &operator=(Field &&) = delete;

  friend class Project;
  Field(hidden, const string &name, const shared_ptr<Project> &project,
        const shared_ptr<Configuration> &configuration,
        const shared_ptr<Manifold> &manifold,
        const shared_ptr<TangentSpace> &tangentspace,
        const shared_ptr<TensorType> &tensortype)
      : Common(name), project(project), configuration(configuration),
        manifold(manifold), tangentspace(tangentspace), tensortype(tensortype) {
  }
  Field(hidden) : Common(hidden()) {}

private:
  static shared_ptr<Field>
  create(const string &name, const shared_ptr<Project> &project,
         const shared_ptr<Configuration> &configuration,
         const shared_ptr<Manifold> &manifold,
         const shared_ptr<TangentSpace> &tangentspace,
         const shared_ptr<TensorType> &tensortype) {
    auto field = make_shared<Field>(hidden(), name, project, configuration,
                                    manifold, tangentspace, tensortype);
    configuration->insert(name, field);
    manifold->insert(name, field);
    tangentspace->insert(name, field);
    tensortype->noinsert(field);
    return field;
  }
  static shared_ptr<Field> create(const H5::CommonFG &loc, const string &entry,
                                  const shared_ptr<Project> &project) {
    auto field = make_shared<Field>(hidden());
    field->read(loc, entry, project);
    return field;
  }
  void read(const H5::CommonFG &loc, const string &entry,
            const shared_ptr<Project> &project);

public:
  virtual ~Field() {}

  virtual ostream &output(ostream &os, int level = 0) const;
  friend ostream &operator<<(ostream &os, const Field &field) {
    return field.output(os);
  }
  virtual void write(const H5::CommonFG &loc,
                     const H5::H5Location &parent) const;

  shared_ptr<DiscreteField>
  createDiscreteField(const string &name,
                      const shared_ptr<Configuration> &configuration,
                      const shared_ptr<Discretization> &discretization,
                      const shared_ptr<Basis> &basis);
  shared_ptr<DiscreteField> readDiscreteField(const H5::CommonFG &loc,
                                              const string &entry);

private:
  friend class CoordinateField;
  void noinsert(const shared_ptr<CoordinateField> &coordinatefield) {}
};
}

#define FIELD_HPP_DONE
#endif // #ifndef FIELD_HPP
#ifndef FIELD_HPP_DONE
#error "Cyclic include depencency"
#endif
