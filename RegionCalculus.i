// SWIG interface file

%module RegionCalculus

%{
#include "RegionCalculus.hpp"
#include <sstream>

typedef RegionCalculus::box<int,3> box3;
typedef RegionCalculus::region<int,3> region3;

typedef RegionCalculus::dbox<int> ibox;
typedef RegionCalculus::dregion<int> iregion;
%}

%include <std_string.i>
%include <std_vector.i>

struct box3;
struct region3;
struct ibox;
struct iregion;

%template(vector_int) std::vector<int>;
%template(vector_box3) std::vector<box3>;
%template(vector_ibox) std::vector<ibox>;

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

%rename("union") ibox::setunion(const ibox& b) const;
struct ibox {
  ibox();
  ibox(int d);
  ibox(const std::vector<int>& lo, const std::vector<int>& hi);

  // Predicates
  bool valid() const;
  int rank() const;
  bool empty() const;
  std::vector<int> lower() const;
  std::vector<int> upper() const;
  std::vector<int> shape() const;
  int size() const;

  // Shift and scale operators
  ibox& operator>>=(const std::vector<int>& p);
  ibox& operator<<=(const std::vector<int>& p);
  ibox& operator*=(const std::vector<int>& p);
  ibox operator>>(const std::vector<int>& p) const;
  ibox operator<<(const std::vector<int>& p) const;
  ibox operator*(const std::vector<int>& p) const;

  // Comparison operators
  bool operator==(const ibox& b) const;
  bool operator!=(const ibox& b) const;
  bool operator<=(const ibox& b) const;
  bool operator>=(const ibox& b) const;
  bool operator<(const ibox& b) const;
  bool operator>(const ibox& b) const;
  bool contains(const std::vector<int>& p) const;
  bool isdisjoint(const ibox& b) const;
  bool issubset(const ibox& b) const;
  bool issuperset(const ibox& b) const;
  bool is_strict_subset(const ibox& b) const;
  bool is_strict_superset(const ibox& b) const;

  // Set operations
  ibox bounding_box(const ibox& b) const;
  ibox operator&(const ibox& b) const;
  ibox intersection(const ibox& b) const;

  // Output
  %extend {
    std::string __str__() const {
      std::ostringstream buf;
      buf << *self;
      return buf.str();
    }
  }
};

%rename("union") iregion::setunion(const iregion& r) const;
struct iregion {
  iregion();
  iregion(int d);
  iregion(const ibox& b);
  iregion(const std::vector<ibox>& bs);
  %extend {
    std::vector<ibox> boxes() const {
      std::vector<ibox> rs = *self;
      return rs;
    }
  }

  // Predicates
  bool valid() const;
  int rank() const;
  bool empty() const;
  int size() const;

  // Set operations
  iregion bounding_box() const;
  iregion operator&(const iregion& r) const;
  iregion operator-(const iregion& r) const;
  iregion operator|(const iregion& r) const;
  iregion operator^(const iregion& r) const;
  iregion intersection(const iregion& r) const;
  iregion difference(const iregion& r) const;
  iregion setunion(const iregion& r) const;
  iregion symmetric_difference(const iregion& r) const;

  // Set comparison operators
  bool contains(const std::vector<int>& p) const;
  bool isdisjoint(const iregion& r) const;

  // Comparison operators
  bool operator<=(const iregion& r) const;
  bool operator>=(const iregion& r) const;
  bool operator<(const iregion& r) const;
  bool operator>(const iregion& r) const;
  bool issubset(const iregion& r) const;
  bool issuperset(const iregion& r) const;
  bool is_strict_subset(const iregion& r) const;
  bool is_strict_superset(const iregion& r) const;
  bool operator==(const iregion& r) const;
  bool operator!=(const iregion& r) const;

  // Output
  %extend {
    std::string __str__() const {
      std::ostringstream buf;
      buf << *self;
      return buf.str();
    }
  }
};
