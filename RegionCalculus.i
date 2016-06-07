// SWIG interface file

%module RegionCalculus

%{
  #include "RegionCalculus.hpp"
  #include <sstream>

  using namespace RegionCalculus;

  typedef RegionCalculus::box<long long, 1> box1;
  typedef RegionCalculus::box<long long, 2> box2;
  typedef RegionCalculus::box<long long, 3> box3;
  typedef RegionCalculus::box<long long, 4> box4;
  typedef RegionCalculus::region<long long, 1> region1;
  typedef RegionCalculus::region<long long, 2> region2;
  typedef RegionCalculus::region<long long, 3> region3;
  typedef RegionCalculus::region<long long, 4> region4;
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

struct point_t;
struct box_t;
struct region_t;

%template(vector_long_long) std::vector<long long>;
%template(vector_box1) std::vector<box1>;
%template(vector_box2) std::vector<box2>;
%template(vector_box3) std::vector<box3>;
%template(vector_box4) std::vector<box4>;
%template(vector_box_t) std::vector<box_t>;

%rename("union") box1::setunion(const box1& b) const;
struct box1 {
  box1();
  box1(const std::vector<long long>& lo, const std::vector<long long>& hi);
  bool empty() const;
  std::vector<long long> shape() const;
  long long size() const;
  box1 bounding_box(const box1& b) const;

  bool operator==(const box1& b) const;
  bool operator!=(const box1& b) const;
  bool operator<=(const box1& b) const;
  bool operator>=(const box1& b) const;
  bool operator<(const box1& b) const;
  bool operator>(const box1& b) const;

  bool contains(const std::vector<long long>& p) const;
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
  box2(const std::vector<long long>& lo, const std::vector<long long>& hi);
  bool empty() const;
  std::vector<long long> shape() const;
  long long size() const;
  box2 bounding_box(const box2& b) const;

  bool operator==(const box2& b) const;
  bool operator!=(const box2& b) const;
  bool operator<=(const box2& b) const;
  bool operator>=(const box2& b) const;
  bool operator<(const box2& b) const;
  bool operator>(const box2& b) const;

  bool contains(const std::vector<long long>& p) const;
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
  box3(const std::vector<long long>& lo, const std::vector<long long>& hi);
  bool empty() const;
  std::vector<long long> shape() const;
  long long size() const;
  box3 bounding_box(const box3& b) const;

  bool operator==(const box3& b) const;
  bool operator!=(const box3& b) const;
  bool operator<=(const box3& b) const;
  bool operator>=(const box3& b) const;
  bool operator<(const box3& b) const;
  bool operator>(const box3& b) const;

  bool contains(const std::vector<long long>& p) const;
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
  box4(const std::vector<long long>& lo, const std::vector<long long>& hi);
  bool empty() const;
  std::vector<long long> shape() const;
  long long size() const;
  box4 bounding_box(const box4& b) const;

  bool operator==(const box4& b) const;
  bool operator!=(const box4& b) const;
  bool operator<=(const box4& b) const;
  bool operator>=(const box4& b) const;
  bool operator<(const box4& b) const;
  bool operator>(const box4& b) const;

  bool contains(const std::vector<long long>& p) const;
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
  long long size() const;
  box1 bounding_box() const;

  bool operator==(const region1& r) const;
  bool operator!=(const region1& r) const;
  bool operator<=(const region1& r) const;
  bool operator>=(const region1& r) const;
  bool operator<(const region1& r) const;
  bool operator>(const region1& r) const;

  bool contains(const std::vector<long long>& p) const;
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
  long long size() const;
  box2 bounding_box() const;

  bool operator==(const region2& r) const;
  bool operator!=(const region2& r) const;
  bool operator<=(const region2& r) const;
  bool operator>=(const region2& r) const;
  bool operator<(const region2& r) const;
  bool operator>(const region2& r) const;

  bool contains(const std::vector<long long>& p) const;
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
  long long size() const;
  box3 bounding_box() const;

  bool operator==(const region3& r) const;
  bool operator!=(const region3& r) const;
  bool operator<=(const region3& r) const;
  bool operator>=(const region3& r) const;
  bool operator<(const region3& r) const;
  bool operator>(const region3& r) const;

  bool contains(const std::vector<long long>& p) const;
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
  long long size() const;
  box4 bounding_box() const;

  bool operator==(const region4& r) const;
  bool operator!=(const region4& r) const;
  bool operator<=(const region4& r) const;
  bool operator>=(const region4& r) const;
  bool operator<(const region4& r) const;
  bool operator>(const region4& r) const;

  bool contains(const std::vector<long long>& p) const;
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

struct point_t {
  point_t();
  point_t(int d);
  point_t(const std::vector<long long>& p);
  // operator std::vector<long long>() const;

  // Access and conversion
  // T operator[](int d) const;
  // T& operator[](int d);
  %extend {
    long long __getitem__(int d) const {
      return (*self)[d];
    }
    void __setitem__(int d, long long x) {
      (*self)[d] = x;
    }
  }
  point_t subpoint(int dir) const;
  point_t superpoint(int dir, long long x) const;

  // Predicates
  bool valid() const;
  void reset();
  int rank() const;
  %extend {
    int __len__() const {
      return self->rank();
    }
  }

  // Unary operators
  point_t operator+() const;
  point_t operator-() const;
  point_t operator~() const;
  // dpoint<bool> operator!() const;

  // Binary operators
  point_t operator+(const point_t &p) const;
  point_t operator-(const point_t &p) const;
  point_t operator*(const point_t &p) const;
  point_t operator/(const point_t &p) const;
  point_t operator%(const point_t &p) const;
  point_t operator&(const point_t &p) const;
  point_t operator|(const point_t &p) const;
  point_t operator^(const point_t &p) const;
  // dpoint<bool> operator&&(const dpoint &p) const;
  // dpoint<bool> operator||(const dpoint &p) const;

  // Unary functions
  point_t abs() const;

  // Binary functions
  point_t min(const point_t &p) const;
  point_t max(const point_t &p) const;

  // // Comparison operators
  // dpoint<bool> operator==(const point_t &p) const;
  // dpoint<bool> operator!=(const point_t &p) const;
  // dpoint<bool> operator<(const point_t &p) const;
  // dpoint<bool> operator>(const point_t &p) const;
  // dpoint<bool> operator<=(const point_t &p) const;
  // dpoint<bool> operator>=(const point_t &p) const;

  bool equal_to(const point_t &p) const;
  %extend {
    bool __eq__(const point_t &p) const { return self->equal_to(p); }
    bool __ne__(const point_t &p) const { return !self->equal_to(p); }
  }
  bool less(const point_t &p) const;
  %extend {
    bool __lt__(const point_t &p) const { return self->less(p); }
    bool __gt__(const point_t &p) const { return p.less(*self); }
    bool __le__(const point_t &p) const { return !p.less(*self); }
    bool __ge__(const point_t &p) const { return !self->less(p); }
  }

  // Reductions
  bool all() const;
  bool any() const;
  long long minval() const;
  long long maxval() const;
  long long sum() const;
  long long prod() const;

  // Output
  %extend {
    std::string __str__() const {
      std::ostringstream buf;
      buf << *self;
      return buf.str();
    }
  }
};

%rename("union") box_t::setunion(const box_t& b) const;
struct box_t {
  box_t();
  box_t(int d);
  box_t(const point_t& lo, const point_t& hi);

  // Predicates
  bool valid() const;
  void reset();
  int rank() const;
  bool empty() const;
  point_t lower() const;
  point_t upper() const;
  point_t shape() const;
  long long size() const;

  // Shift and scale operators
  box_t& operator>>=(const point_t& p);
  box_t& operator<<=(const point_t& p);
  box_t& operator*=(const point_t& p);
  box_t operator>>(const point_t& p) const;
  box_t operator<<(const point_t& p) const;
  box_t operator*(const point_t& p) const;

  // Comparison operators
  bool operator==(const box_t& b) const;
  bool operator!=(const box_t& b) const;
  bool operator<=(const box_t& b) const;
  bool operator>=(const box_t& b) const;
  bool operator<(const box_t& b) const;
  bool operator>(const box_t& b) const;
  bool contains(const point_t& p) const;
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
      buf << *self;
      return buf.str();
    }
  }
};

%rename("union") region_t::setunion(const region_t& r) const;
struct region_t {
  region_t();
  region_t(int d);
  region_t(const box_t& b);
  region_t(const std::vector<box_t>& bs);
  %extend {
    std::vector<box_t> boxes() const {
      std::vector<box_t> rs = *self;
      return rs;
    }
  }

  // Predicates
  bool valid() const;
  void reset();
  int rank() const;
  bool empty() const;
  long long size() const;

  // Set operations
  box_t bounding_box() const;
  region_t operator&(const region_t& r) const;
  region_t operator-(const region_t& r) const;
  region_t operator|(const region_t& r) const;
  region_t operator^(const region_t& r) const;
  region_t intersection(const region_t& r) const;
  region_t difference(const region_t& r) const;
  region_t setunion(const region_t& r) const;
  region_t symmetric_difference(const region_t& r) const;

  // Set comparison operators
  bool contains(const point_t& p) const;
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
      buf << *self;
      return buf.str();
    }
  }
};
