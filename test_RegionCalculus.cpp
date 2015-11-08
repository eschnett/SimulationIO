#include "RegionCalculus.hpp"

#include <gtest/gtest.h>

#include <sstream>

using std::ostringstream;

using namespace RegionCalculus;

TEST(RegionCalculus, point) {
  typedef point<int, 3> point;
  point p;
  EXPECT_FALSE(all(p));
  EXPECT_FALSE(any(p));
  point p0(0);
  EXPECT_FALSE(all(p0));
  EXPECT_FALSE(any(p0));
  EXPECT_TRUE(all(p0 == p));
  point p1(::point<bool, 3>(true));
  EXPECT_TRUE(all(p1));
  EXPECT_TRUE(any(p1));
  EXPECT_TRUE(all(p1 != p));
  EXPECT_TRUE(all(p1 == +p1));
  EXPECT_TRUE(all(p0 == -p0));
  EXPECT_TRUE(all(p1 == abs(-p1)));
  EXPECT_TRUE(all(-p1 == ~p0));
  EXPECT_TRUE(all(!p0));
  point p2(point(2));
  EXPECT_TRUE(all(p1 + p1 == p2));
  EXPECT_TRUE(all(p2 - p1 == p1));
  EXPECT_TRUE(all(p2 * p1 == p2));
  EXPECT_TRUE(all((p2 & p1) == p0));
  EXPECT_TRUE(all((p2 | p1) == p1 + p2));
  EXPECT_TRUE(all((p2 ^ p1) == p1 + p2));
  EXPECT_TRUE(all(p1 && p2));
  EXPECT_TRUE(all(p1 || p2));
  EXPECT_TRUE(all(p0 <= p1 && p0 < p1 && p1 >= p0 && p1 > p0));
  EXPECT_EQ(2, minval(p2));
  EXPECT_EQ(2, maxval(p2));
  EXPECT_EQ(3, sum(p1));
  EXPECT_EQ(8, prod(p2));
  point p3;
  p3.elt[0] = 0;
  p3.elt[1] = 1;
  p3.elt[2] = 2;
  EXPECT_TRUE(all((p1 + p3) * point(2) - p3 == p3 + p2));
  ostringstream buf;
  buf << p3;
  EXPECT_EQ("[0,1,2]", buf.str());
}

TEST(RegionCalculus, box) {
  typedef point<int, 3> point;
  typedef box<int, 3> box;
  box b;
  EXPECT_TRUE(b.empty());
  point p0(0), p1(1);
  box b1(box(p0, p1));
  EXPECT_EQ(1, b1.size());
  box b4(p0, point(4));
  box b5 = b4 >> p1;
  box b3 = b4 << p1;
  box b6 = b3 * point(2);
  EXPECT_TRUE(b4 == b4);
  EXPECT_TRUE(b4 != b5);
  EXPECT_TRUE(b4.contains(p1));
  EXPECT_FALSE(b5.contains(p0));
  EXPECT_FALSE(b5.isdisjoint(b3));
  EXPECT_TRUE(b1.isdisjoint(b5));
  EXPECT_TRUE(b.isdisjoint(b5));
  EXPECT_TRUE(b1.isdisjoint(b));
  EXPECT_TRUE(b6.issuperset(b3));
  EXPECT_FALSE(b3.issuperset(b6));
  EXPECT_TRUE(b4.issuperset(b4));
  EXPECT_TRUE(b4.issuperset(b));
  EXPECT_FALSE(b.issuperset(b4));
  EXPECT_TRUE(b.issuperset(b));
  EXPECT_FALSE(b4.is_strict_superset(b4));
  EXPECT_TRUE(b4.is_strict_superset(b));
  EXPECT_FALSE(b.is_strict_superset(b4));
  EXPECT_FALSE(b.is_strict_superset(b));
  EXPECT_EQ(box(p1, point(3)), b5.intersection(b3));
  EXPECT_TRUE(b1.intersection(b5).empty());
  EXPECT_TRUE(b.intersection(b5).empty());
  EXPECT_TRUE(b1.intersection(b).empty());
  ostringstream buf;
  buf << b4;
  EXPECT_EQ("([0,0,0]:[4,4,4])", buf.str());
}

TEST(RegionCalculus, region) {
  typedef point<int, 3> point;
  typedef box<int, 3> box;
  typedef region<int, 3> region;
  region r;
  EXPECT_TRUE(r.invariant());
  EXPECT_TRUE(r.empty());
  point p;
  point p1(1);
  box b;
  box b1(p, p1);
  region r0(b);
  EXPECT_TRUE(r0.invariant());
  EXPECT_TRUE(r0.empty());
  region r1(b1);
  EXPECT_TRUE(r1.invariant());
  EXPECT_FALSE(r1.empty());
  EXPECT_TRUE(r == r);
  EXPECT_TRUE(r1 == r1);
  EXPECT_FALSE(r != r);
  EXPECT_TRUE(r != r1);
  point p2(2);
  box b2(p, p2);
  region r2(b2);
  EXPECT_TRUE(r == r.intersection(r1));
  EXPECT_TRUE(r == r1.intersection(r));
  EXPECT_TRUE(r1 == r1.intersection(r2));
  EXPECT_TRUE(r1 == r2.intersection(r1));
  EXPECT_TRUE(r == r.difference(r));
  EXPECT_TRUE(r1 == r1.difference(r));
  EXPECT_TRUE(r == r.difference(r1));
  EXPECT_TRUE(r == r1.difference(r1));
  EXPECT_TRUE(r == r.setunion(r));
  EXPECT_TRUE(r1 == r1.setunion(r));
  EXPECT_TRUE(r1 == r.setunion(r1));
  EXPECT_TRUE(r1 == r1.setunion(r1));
  EXPECT_TRUE(r == r.symmetric_difference(r));
  EXPECT_TRUE(r1 == r1.symmetric_difference(r));
  EXPECT_TRUE(r1 == r.symmetric_difference(r1));
  EXPECT_TRUE(r == r1.symmetric_difference(r1));
  vector<region> rs;
  rs.push_back(r);
  rs.push_back(r1);
  rs.push_back(r2);
  box b4(p, point(4));
  region r4(b4);
  rs.push_back(r4);
  region r12;
  r12.boxes.push_back(b1);
  r12.boxes.push_back(box(p1, p2));
  rs.push_back(r12);
  rs.push_back(r2.difference(r1));
  rs.push_back(r2.symmetric_difference(r12));
  for (std::size_t i = 0; i < rs.size(); ++i) {
    const auto &ri = rs[i];
    EXPECT_TRUE(ri.invariant());
    for (std::size_t j = 0; j < rs.size(); ++j) {
      const auto &rj = rs[j];
      auto rintersection = ri.intersection(rj);
      auto rdifference = ri.difference(rj);
      auto rsetunion = ri.setunion(rj);
      auto rsymmetric_difference = ri.symmetric_difference(rj);
      EXPECT_TRUE(rintersection.invariant());
      EXPECT_TRUE(rdifference.invariant());
      EXPECT_TRUE(rsetunion.invariant());
      EXPECT_TRUE(rsymmetric_difference.invariant());
      EXPECT_TRUE(ri.issuperset(rintersection));
      EXPECT_TRUE(rj.issuperset(rintersection));
      EXPECT_TRUE(ri.issuperset(rdifference));
      EXPECT_TRUE(rsetunion.issuperset(ri));
      EXPECT_TRUE(rsetunion.issuperset(rj));
      EXPECT_TRUE(rintersection.isdisjoint(rsymmetric_difference));
      EXPECT_TRUE(rdifference.isdisjoint(rj));
      EXPECT_TRUE(rsetunion.issuperset(rintersection));
      EXPECT_TRUE(rsetunion.issuperset(rdifference));
      EXPECT_TRUE(rsetunion.issuperset(rsymmetric_difference));
      EXPECT_TRUE(rintersection.setunion(rsymmetric_difference) == rsetunion);
      EXPECT_TRUE(rdifference.setunion(rj) == rsetunion);
      EXPECT_TRUE(rintersection == rj.intersection(ri));
      EXPECT_TRUE(rsetunion == rj.setunion(ri));
      EXPECT_TRUE(rsymmetric_difference == rj.symmetric_difference(ri));
      if (ri == rj) {
        EXPECT_TRUE(rintersection == ri);
        EXPECT_TRUE(rdifference.empty());
        EXPECT_TRUE(rsetunion == ri);
        EXPECT_TRUE(rsymmetric_difference.empty());
      }
    }
  }
  ostringstream buf;
  buf << r12;
  EXPECT_EQ("{([0,0,0]:[1,1,1]),([1,1,1]:[2,2,2])}", buf.str());
}

TEST(RegionCalculus, dpoint) {
  const int dim = 3;
  typedef dpoint<int> dpoint;
  dpoint p(dim);
  EXPECT_FALSE(all(p));
  EXPECT_FALSE(any(p));
  dpoint p0(dim, 0);
  EXPECT_FALSE(all(p0));
  EXPECT_FALSE(any(p0));
  EXPECT_TRUE(all(p0 == p));
  dpoint p1(::dpoint<bool>(dim, true));
  EXPECT_TRUE(all(p1));
  EXPECT_TRUE(any(p1));
  EXPECT_TRUE(all(p1 != p));
  EXPECT_TRUE(all(p1 == +p1));
  EXPECT_TRUE(all(p0 == -p0));
  EXPECT_TRUE(all(p1 == abs(-p1)));
  EXPECT_TRUE(all(-p1 == ~p0));
  EXPECT_TRUE(all(!p0));
  dpoint p2(dpoint(dim, 2));
  EXPECT_TRUE(all(p1 + p1 == p2));
  EXPECT_TRUE(all(p2 - p1 == p1));
  EXPECT_TRUE(all(p2 * p1 == p2));
  EXPECT_TRUE(all((p2 & p1) == p0));
  EXPECT_TRUE(all((p2 | p1) == p1 + p2));
  EXPECT_TRUE(all((p2 ^ p1) == p1 + p2));
  EXPECT_TRUE(all(p1 && p2));
  EXPECT_TRUE(all(p1 || p2));
  EXPECT_TRUE(all(p0 <= p1 && p0 < p1 && p1 >= p0 && p1 > p0));
  EXPECT_EQ(2, minval(p2));
  EXPECT_EQ(2, maxval(p2));
  EXPECT_EQ(3, sum(p1));
  EXPECT_EQ(8, prod(p2));
  vector<int> p3vals(dim);
  p3vals[0] = 0;
  p3vals[1] = 1;
  p3vals[2] = 2;
  dpoint p3(p3vals);
  EXPECT_TRUE(all((p1 + p3) * dpoint(dim, 2) - p3 == p3 + p2));
  ostringstream buf;
  buf << p3;
  EXPECT_EQ("[0,1,2]", buf.str());
}

TEST(RegionCalculus, dbox) {
  const int dim = 3;
  typedef dpoint<int> dpoint;
  typedef dbox<int> dbox;
  dbox b(dim);
  EXPECT_TRUE(b.empty());
  dpoint p0(dim, 0), p1(dim, 1);
  dbox b1(dbox(p0, p1));
  EXPECT_EQ(1, b1.size());
  dbox b4(p0, dpoint(dim, 4));
  dbox b5 = b4 >> p1;
  dbox b3 = b4 << p1;
  dbox b6 = b3 * dpoint(dim, 2);
  EXPECT_TRUE(b4 == b4);
  EXPECT_TRUE(b4 != b5);
  EXPECT_TRUE(b4.contains(p1));
  EXPECT_FALSE(b5.contains(p0));
  EXPECT_FALSE(b5.isdisjoint(b3));
  EXPECT_TRUE(b1.isdisjoint(b5));
  EXPECT_TRUE(b.isdisjoint(b5));
  EXPECT_TRUE(b1.isdisjoint(b));
  EXPECT_TRUE(b6.issuperset(b3));
  EXPECT_FALSE(b3.issuperset(b6));
  EXPECT_TRUE(b4.issuperset(b4));
  EXPECT_TRUE(b4.issuperset(b));
  EXPECT_FALSE(b.issuperset(b4));
  EXPECT_TRUE(b.issuperset(b));
  EXPECT_FALSE(b4.is_strict_superset(b4));
  EXPECT_TRUE(b4.is_strict_superset(b));
  EXPECT_FALSE(b.is_strict_superset(b4));
  EXPECT_FALSE(b.is_strict_superset(b));
  EXPECT_EQ(dbox(p1, dpoint(dim, 3)), b5.intersection(b3));
  EXPECT_TRUE(b1.intersection(b5).empty());
  EXPECT_TRUE(b.intersection(b5).empty());
  EXPECT_TRUE(b1.intersection(b).empty());
  ostringstream buf;
  buf << b4;
  EXPECT_EQ("([0,0,0]:[4,4,4])", buf.str());
}

TEST(RegionCalculus, dregion) {
  const int dim = 3;
  typedef dpoint<int> dpoint;
  typedef dbox<int> dbox;
  typedef dregion<int> dregion;
  dregion r(dim);
  EXPECT_TRUE(r.invariant());
  EXPECT_TRUE(r.empty());
  dpoint p(dim);
  dpoint p1(dim, 1);
  dbox b(dim);
  dbox b1(p, p1);
  dregion r0(b);
  EXPECT_TRUE(r0.invariant());
  EXPECT_TRUE(r0.empty());
  dregion r1(b1);
  EXPECT_TRUE(r1.invariant());
  EXPECT_FALSE(r1.empty());
  EXPECT_TRUE(r == r);
  EXPECT_TRUE(r1 == r1);
  EXPECT_FALSE(r != r);
  EXPECT_TRUE(r != r1);
  dpoint p2(dim, 2);
  dbox b2(p, p2);
  dregion r2(b2);
  EXPECT_TRUE(r == r.intersection(r1));
  EXPECT_TRUE(r == r1.intersection(r));
  EXPECT_TRUE(r1 == r1.intersection(r2));
  EXPECT_TRUE(r1 == r2.intersection(r1));
  EXPECT_TRUE(r == r.difference(r));
  EXPECT_TRUE(r1 == r1.difference(r));
  EXPECT_TRUE(r == r.difference(r1));
  EXPECT_TRUE(r == r1.difference(r1));
  EXPECT_TRUE(r == r.setunion(r));
  EXPECT_TRUE(r1 == r1.setunion(r));
  EXPECT_TRUE(r1 == r.setunion(r1));
  EXPECT_TRUE(r1 == r1.setunion(r1));
  EXPECT_TRUE(r == r.symmetric_difference(r));
  EXPECT_TRUE(r1 == r1.symmetric_difference(r));
  EXPECT_TRUE(r1 == r.symmetric_difference(r1));
  EXPECT_TRUE(r == r1.symmetric_difference(r1));
  vector<dregion> rs;
  rs.push_back(r);
  rs.push_back(r1);
  rs.push_back(r2);
  dbox b4(p, dpoint(dim, 4));
  dregion r4(b4);
  rs.push_back(r4);
  vector<dbox> r12vals;
  r12vals.push_back(b1);
  r12vals.push_back(dbox(p1, p2));
  dregion r12(r12vals);
  rs.push_back(r12);
  rs.push_back(r2.difference(r1));
  rs.push_back(r2.symmetric_difference(r12));
  for (std::size_t i = 0; i < rs.size(); ++i) {
    const auto &ri = rs[i];
    EXPECT_TRUE(ri.invariant());
    for (std::size_t j = 0; j < rs.size(); ++j) {
      const auto &rj = rs[j];
      auto rintersection = ri.intersection(rj);
      auto rdifference = ri.difference(rj);
      auto rsetunion = ri.setunion(rj);
      auto rsymmetric_difference = ri.symmetric_difference(rj);
      EXPECT_TRUE(rintersection.invariant());
      EXPECT_TRUE(rdifference.invariant());
      EXPECT_TRUE(rsetunion.invariant());
      EXPECT_TRUE(rsymmetric_difference.invariant());
      EXPECT_TRUE(ri.issuperset(rintersection));
      EXPECT_TRUE(rj.issuperset(rintersection));
      EXPECT_TRUE(ri.issuperset(rdifference));
      EXPECT_TRUE(rsetunion.issuperset(ri));
      EXPECT_TRUE(rsetunion.issuperset(rj));
      EXPECT_TRUE(rintersection.isdisjoint(rsymmetric_difference));
      EXPECT_TRUE(rdifference.isdisjoint(rj));
      EXPECT_TRUE(rsetunion.issuperset(rintersection));
      EXPECT_TRUE(rsetunion.issuperset(rdifference));
      EXPECT_TRUE(rsetunion.issuperset(rsymmetric_difference));
      EXPECT_TRUE(rintersection.setunion(rsymmetric_difference) == rsetunion);
      EXPECT_TRUE(rdifference.setunion(rj) == rsetunion);
      EXPECT_TRUE(rintersection == rj.intersection(ri));
      EXPECT_TRUE(rsetunion == rj.setunion(ri));
      EXPECT_TRUE(rsymmetric_difference == rj.symmetric_difference(ri));
      if (ri == rj) {
        EXPECT_TRUE(rintersection == ri);
        EXPECT_TRUE(rdifference.empty());
        EXPECT_TRUE(rsetunion == ri);
        EXPECT_TRUE(rsymmetric_difference.empty());
      }
    }
  }
  ostringstream buf;
  buf << r12;
  EXPECT_EQ("{([0,0,0]:[1,1,1]),([1,1,1]:[2,2,2])}", buf.str());
}

#include "src/gtest_main.cc"
