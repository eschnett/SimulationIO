// SWIG interface file

%module RegionCalculus

%{
#include "RegionCalculus.hpp"
#include <sstream>

typedef RegionCalculus::box<int,3> box3;
typedef RegionCalculus::region<int,3> region3;

typedef RegionCalculus::dbox<int> box_t;
typedef RegionCalculus::dregion<int> region_t;
%}

%include <std_string.i>
%include <std_vector.i>

struct box3;
struct region3;
struct box_t;
struct region_t;

%template(vector_int) std::vector<int>;
%template(vector_box3) std::vector<box3>;
%template(vector_box_t) std::vector<box_t>;

%rename("union") box3::setunion(const box3& b) const;
struct box3 {
  box3();
  box3(const std::vector<int>& lo, const std::vector<int>& hi);
  bool empty() const;
  std::vector<int> shape() const;
  int size() const;
  box3 bounding_box(const box3& b) const;

  bool operator==(const box3& b) const;
  bool operator!=(const box3& b) const;
  bool operator<=(const box3& b) const;
  bool operator>=(const box3& b) const;
  bool operator<(const box3& b) const;
  bool operator>(const box3& b) const;

  bool contains(const std::vector<int>& p) const;
  bool isdisjoint(const box3& b) const;
  bool issubset(const box3& b) const;
  bool issuperset(const box3& b) const;
  bool is_strict_subset(const box3& b) const;
  bool is_strict_superset(const box3& b) const;

  box3 operator&(const box3& b) const;
  box3 intersection(const box3& b) const;
  std::vector<box3> operator-(const box3& b) const;
  std::vector<box3> difference(const box3& b) const;
  std::vector<box3> operator|(const box3& b) const;
  std::vector<box3> setunion(const box3& b) const;
  std::vector<box3> operator^(const box3& b) const;
  std::vector<box3> symmetric_difference(const box3& b) const;
};

%rename("union") region3::setunion(const region3& r) const;
struct region3 {
  region3();
  region3(const box3& b);
  bool empty() const;
  int size() const;
  box3 bounding_box() const;

  bool operator==(const region3& r) const;
  bool operator!=(const region3& r) const;
  bool operator<=(const region3& r) const;
  bool operator>=(const region3& r) const;
  bool operator<(const region3& r) const;
  bool operator>(const region3& r) const;

  bool contains(const std::vector<int>& p) const;
  bool isdisjoint(const box3& b) const;
  bool issubset(const box3& b) const;
  bool issuperset(const box3& b) const;
  bool is_strict_subset(const box3& b) const;
  bool is_strict_superset(const box3& b) const;
  bool isdisjoint(const region3& r) const;
  bool issubset(const region3& r) const;
  bool issuperset(const region3& r) const;
  bool is_strict_subset(const region3& r) const;
  bool is_strict_superset(const region3& r) const;

  region3 operator&(const region3& r) const;
  region3 intersection(const region3& r) const;
  region3 operator-(const region3& r) const;
  region3 difference(const region3& r) const;
  region3 operator|(const region3& r) const;
  region3 setunion(const region3& r) const;
  region3 operator^(const region3& r) const;
  region3 symmetric_difference(const region3& r) const;
};

%rename("union") box_t::setunion(const region3& r) const;
struct box_t {
  box_t();
  box_t(int d);
  box_t(const std::vector<int>& lo, const std::vector<int>& hi);

  // Predicates
  %extend {
    bool valid() const { return bool(self); }
  }
  bool empty() const;
  std::vector<int> lower() const;
  std::vector<int> upper() const;
  std::vector<int> shape() const;
  int size() const;

  // Shift and scale operators
  box_t& operator>>=(const std::vector<int>& p);
  box_t& operator<<=(const std::vector<int>& p);
  box_t& operator*=(const std::vector<int>& p);
  box_t operator>>(const std::vector<int>& p) const;
  box_t operator<<(const std::vector<int>& p) const;
  box_t operator*(const std::vector<int>& p) const;

  // Comparison operators
  bool operator==(const box_t& b) const;
  bool operator!=(const box_t& b) const;
  bool operator<=(const box_t& b) const;
  bool operator>=(const box_t& b) const;
  bool operator<(const box_t& b) const;
  bool operator>(const box_t& b) const;
  bool contains(const std::vector<int>& p) const;
  bool isdisjoint(const box_t& b) const;
  bool issubset(const box_t& b) const;
  bool issuperset(const box_t& b) const;
  bool is_strict_subset(const box_t& b) const;
  bool is_strict_superset(const box_t& b) const;

  // Set operations
  box_t bounding_box(const box_t& b) const;
  box_t operator&(const box_t& b) const;
  box_t intersection(const box_t& b) const;

  // Output
  %extend {
    std::string __str__() const {
      std::ostringstream buf;
      buf << self;
      return buf.str();
    }
  }
};

%rename("union") box_t::setunion(const box_t& b) const;
struct region_t {
  region_t();
  region_t(int d);
  region_t(const box_t& b);
  region_t(const std::vector<box_t>& bs);

  // Predicates
  %extend {
    bool valid() const { return bool(self); }
  }
  bool empty() const;
  int size() const;

  // Set operations
  region_t bounding_box() const;
  region_t operator&(const region_t& r) const;
  region_t operator-(const region_t& r) const;
  region_t operator|(const region_t& r) const;
  region_t operator^(const region_t& r) const;
  region_t intersection(const region_t& r) const;
  region_t difference(const region_t& r) const;
  region_t setunion(const region_t& r) const;
  region_t symmetric_difference(const region_t& r) const;

  // Set comparison operators
  bool contains(const std::vector<int>& p) const;
  bool isdisjoint(const region_t& r) const;

  // Comparison operators
  bool operator<=(const region_t& r) const;
  bool operator>=(const region_t& r) const;
  bool operator<(const region_t& r) const;
  bool operator>(const region_t& r) const;
  bool issubset(const region_t& r) const;
  bool issuperset(const region_t& r) const;
  bool is_strict_subset(const region_t& r) const;
  bool is_strict_superset(const region_t& r) const;
  bool operator==(const region_t& r) const;
  bool operator!=(const region_t& r) const;

  // Output
  %extend {
    std::string __str__() const {
      std::ostringstream buf;
      buf << self;
      return buf.str();
    }
  }
};
