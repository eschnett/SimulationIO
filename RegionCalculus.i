// SWIG interface file

%module RegionCalculus

%{
#include "RegionCalculus.hpp"

typedef RegionCalculus::box<int,3> box3;
typedef RegionCalculus::region<int,3> region3;
%}

%include <std_vector.i>

struct box3;

%template(vector_int) std::vector<int>;
%template(vector_box3) std::vector<box3>;

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

  bool contains(const std::vector<int> &p) const;
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

  bool contains(const std::vector<int> &p) const;
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
