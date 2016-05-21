#ifndef COMMON_HPP
#define COMMON_HPP

#include "LazyPtr.hpp"

#include <H5Cpp.h>

#include <iostream>
#include <string>

namespace SimulationIO {

using LazyPtr::flag;

// using LazyPtr::lazy;
// using LazyPtr::lazy_from_shared;
// using LazyPtr::lazy_ptr;
// using LazyPtr::lazy_weak_ptr;

template <typename T> using lazy_ptr = std::shared_ptr<T>;
template <typename T> using lazy_weak_ptr = std::weak_ptr<T>;

using std::ostream;
using std::string;

// C++ make_shared requires constructors to be public; we add a field of type
// `hidden` to ensure they are not called accidentally.
struct hidden {};

// Entity relationships

#if 0
// Parents and children; children are "owned" by their parents, and only created
// by their parent
template <typename Child> struct children { map<string, Child *> children; };
template <typename Parent> struct parent { Parent *parent; };

// Links that register with their target
template <typename Source> struct backlinks { map<string, Source *> sources; };
template <typename Target> struct link { Target *target; };

// Unidirectionsl links that don't register with their target
template <typename Source> struct nobacklinks {};
template <typename Target> struct unilink { Target *target; };
#endif

// An always empty pseudo-container type indicating that there is no
// back-link
template <typename T> struct NoBackLink {
  constexpr bool nobacklink() const noexcept { return true; }
};

// Common to all file elements

struct Common {
  string name;

  virtual bool invariant() const { return !name.empty(); }

protected:
  Common(const string &name) : name(name) {}
  Common(hidden) {}

public:
  virtual ~Common() {}
  virtual ostream &output(ostream &os, int level = 0) const = 0;
  virtual void write(const H5::CommonFG &loc,
                     const H5::H5Location &parent) const = 0;

  // The association between names and integer values below MUST NOT BE
  // MODIFIED, except that new integer values may be added.
  enum types {
    type_Basis = 1,
    type_BasisVector = 2,
    type_DiscreteField = 3,
    type_DiscreteFieldBlock = 4,
    type_DiscreteFieldBlockComponent = 5,
    type_Discretization = 6,
    type_DiscretizationBlock = 7,
    type_Field = 8,
    type_Manifold = 9,
    type_Project = 10,
    type_TangentSpace = 11,
    type_TensorComponent = 12,
    type_TensorType = 13,
    type_Configuration = 14,
    type_Parameter = 15,
    type_ParameterValue = 16,
    type_CoordinateSystem = 17,
    type_CoordinateField = 18,
    type_SubDiscretization = 19,
  };
};
}

#endif // #ifndef COMMON_HPP
