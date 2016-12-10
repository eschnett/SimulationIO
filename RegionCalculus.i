// SWIG interface file

%module(package="pysimulationio") RegionCalculus

%{
#include "RegionCalculus.hpp"
#include <sstream>

typedef RegionCalculus::box<int,1> box1;
typedef RegionCalculus::box<int,2> box2;
typedef RegionCalculus::box<int,3> box3;
typedef RegionCalculus::box<int,4> box4;
typedef RegionCalculus::region<int,1> region1;
typedef RegionCalculus::region<int,2> region2;
typedef RegionCalculus::region<int,3> region3;
typedef RegionCalculus::region<int,4> region4;

typedef RegionCalculus::dbox<int> ibox;
typedef RegionCalculus::dregion<int> iregion;
%}

%include <std_string.i>
%include <std_vector.i>

struct box1;
struct box2;
struct box3;
struct box4;
struct region1;
struct region2;
struct region3;
struct region4;
struct ibox;
struct iregion;

%template(vector_int) std::vector<int>;
%template(vector_box1) std::vector<box1>;
%template(vector_box2) std::vector<box2>;
%template(vector_box3) std::vector<box3>;
%template(vector_box4) std::vector<box4>;
%template(vector_ibox) std::vector<ibox>;

%rename("union") box1::setunion(const box1& b) const;
struct box1 {
  box1();
  box1(const std::vector<int>& lo, const std::vector<int>& hi);
  bool empty() const;
  std::vector<int> shape() const;
  int size() const;
  box1 bounding_box(const box1& b) const;

  bool operator==(const box1& b) const;
  bool operator!=(const box1& b) const;
  bool operator<=(const box1& b) const;
  bool operator>=(const box1& b) const;
  bool operator<(const box1& b) const;
  bool operator>(const box1& b) const;

  bool contains(const std::vector<int>& p) const;
  bool isdisjoint(const box1& b) const;
  bool issubset(const box1& b) const;
  bool issuperset(const box1& b) const;
  bool is_strict_subset(const box1& b) const;
  bool is_strict_superset(const box1& b) const;

  box1 operator&(const box1& b) const;
  box1 intersection(const box1& b) const;
  std::vector<box1> operator-(const box1& b) const;
  std::vector<box1> difference(const box1& b) const;
  std::vector<box1> operator|(const box1& b) const;
  std::vector<box1> setunion(const box1& b) const;
  std::vector<box1> operator^(const box1& b) const;
  std::vector<box1> symmetric_difference(const box1& b) const;
};

%rename("union") box2::setunion(const box2& b) const;
struct box2 {
  box2();
  box2(const std::vector<int>& lo, const std::vector<int>& hi);
  bool empty() const;
  std::vector<int> shape() const;
  int size() const;
  box2 bounding_box(const box2& b) const;

  bool operator==(const box2& b) const;
  bool operator!=(const box2& b) const;
  bool operator<=(const box2& b) const;
  bool operator>=(const box2& b) const;
  bool operator<(const box2& b) const;
  bool operator>(const box2& b) const;

  bool contains(const std::vector<int>& p) const;
  bool isdisjoint(const box2& b) const;
  bool issubset(const box2& b) const;
  bool issuperset(const box2& b) const;
  bool is_strict_subset(const box2& b) const;
  bool is_strict_superset(const box2& b) const;

  box2 operator&(const box2& b) const;
  box2 intersection(const box2& b) const;
  std::vector<box2> operator-(const box2& b) const;
  std::vector<box2> difference(const box2& b) const;
  std::vector<box2> operator|(const box2& b) const;
  std::vector<box2> setunion(const box2& b) const;
  std::vector<box2> operator^(const box2& b) const;
  std::vector<box2> symmetric_difference(const box2& b) const;
};

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

%rename("union") box4::setunion(const box4& b) const;
struct box4 {
  box4();
  box4(const std::vector<int>& lo, const std::vector<int>& hi);
  bool empty() const;
  std::vector<int> shape() const;
  int size() const;
  box4 bounding_box(const box4& b) const;

  bool operator==(const box4& b) const;
  bool operator!=(const box4& b) const;
  bool operator<=(const box4& b) const;
  bool operator>=(const box4& b) const;
  bool operator<(const box4& b) const;
  bool operator>(const box4& b) const;

  bool contains(const std::vector<int>& p) const;
  bool isdisjoint(const box4& b) const;
  bool issubset(const box4& b) const;
  bool issuperset(const box4& b) const;
  bool is_strict_subset(const box4& b) const;
  bool is_strict_superset(const box4& b) const;

  box4 operator&(const box4& b) const;
  box4 intersection(const box4& b) const;
  std::vector<box4> operator-(const box4& b) const;
  std::vector<box4> difference(const box4& b) const;
  std::vector<box4> operator|(const box4& b) const;
  std::vector<box4> setunion(const box4& b) const;
  std::vector<box4> operator^(const box4& b) const;
  std::vector<box4> symmetric_difference(const box4& b) const;
};

%rename("union") region1::setunion(const region1& r) const;
struct region1 {
  region1();
  region1(const box1& b);
  bool empty() const;
  int size() const;
  box1 bounding_box() const;

  bool operator==(const region1& r) const;
  bool operator!=(const region1& r) const;
  bool operator<=(const region1& r) const;
  bool operator>=(const region1& r) const;
  bool operator<(const region1& r) const;
  bool operator>(const region1& r) const;

  bool contains(const std::vector<int>& p) const;
  bool isdisjoint(const box1& b) const;
  bool issubset(const box1& b) const;
  bool issuperset(const box1& b) const;
  bool is_strict_subset(const box1& b) const;
  bool is_strict_superset(const box1& b) const;
  bool isdisjoint(const region1& r) const;
  bool issubset(const region1& r) const;
  bool issuperset(const region1& r) const;
  bool is_strict_subset(const region1& r) const;
  bool is_strict_superset(const region1& r) const;

  region1 operator&(const region1& r) const;
  region1 intersection(const region1& r) const;
  region1 operator-(const region1& r) const;
  region1 difference(const region1& r) const;
  region1 operator|(const region1& r) const;
  region1 setunion(const region1& r) const;
  region1 operator^(const region1& r) const;
  region1 symmetric_difference(const region1& r) const;
};

%rename("union") region2::setunion(const region2& r) const;
struct region2 {
  region2();
  region2(const box2& b);
  bool empty() const;
  int size() const;
  box2 bounding_box() const;

  bool operator==(const region2& r) const;
  bool operator!=(const region2& r) const;
  bool operator<=(const region2& r) const;
  bool operator>=(const region2& r) const;
  bool operator<(const region2& r) const;
  bool operator>(const region2& r) const;

  bool contains(const std::vector<int>& p) const;
  bool isdisjoint(const box2& b) const;
  bool issubset(const box2& b) const;
  bool issuperset(const box2& b) const;
  bool is_strict_subset(const box2& b) const;
  bool is_strict_superset(const box2& b) const;
  bool isdisjoint(const region2& r) const;
  bool issubset(const region2& r) const;
  bool issuperset(const region2& r) const;
  bool is_strict_subset(const region2& r) const;
  bool is_strict_superset(const region2& r) const;

  region2 operator&(const region2& r) const;
  region2 intersection(const region2& r) const;
  region2 operator-(const region2& r) const;
  region2 difference(const region2& r) const;
  region2 operator|(const region2& r) const;
  region2 setunion(const region2& r) const;
  region2 operator^(const region2& r) const;
  region2 symmetric_difference(const region2& r) const;
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

%rename("union") region4::setunion(const region4& r) const;
struct region4 {
  region4();
  region4(const box4& b);
  bool empty() const;
  int size() const;
  box4 bounding_box() const;

  bool operator==(const region4& r) const;
  bool operator!=(const region4& r) const;
  bool operator<=(const region4& r) const;
  bool operator>=(const region4& r) const;
  bool operator<(const region4& r) const;
  bool operator>(const region4& r) const;

  bool contains(const std::vector<int>& p) const;
  bool isdisjoint(const box4& b) const;
  bool issubset(const box4& b) const;
  bool issuperset(const box4& b) const;
  bool is_strict_subset(const box4& b) const;
  bool is_strict_superset(const box4& b) const;
  bool isdisjoint(const region4& r) const;
  bool issubset(const region4& r) const;
  bool issuperset(const region4& r) const;
  bool is_strict_subset(const region4& r) const;
  bool is_strict_superset(const region4& r) const;

  region4 operator&(const region4& r) const;
  region4 intersection(const region4& r) const;
  region4 operator-(const region4& r) const;
  region4 difference(const region4& r) const;
  region4 operator|(const region4& r) const;
  region4 setunion(const region4& r) const;
  region4 operator^(const region4& r) const;
  region4 symmetric_difference(const region4& r) const;
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
  ibox bounding_box() const;
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
