#include "RegionCalculus.hpp"

#include <gtest/gtest.h>

#include <cstdlib>
#include <functional>
#include <sstream>

using std::equal_to;
using std::ostringstream;

int irand(int imax) { return rand() / (RAND_MAX / imax); }

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
  EXPECT_TRUE(equal_to<point>()(p2, point(::point<long long, 3>(p2))));
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
  box b7 = b4.grow(p0, p1);
  box b8 = b4.grow(p1, p0);
  EXPECT_EQ(b4, (box(::box<long long, 3>(b4))));
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
  EXPECT_TRUE(b7.issuperset(b4));
  EXPECT_TRUE(b7.issuperset(b5));
  EXPECT_TRUE(b8.issuperset(b4));
  EXPECT_TRUE(b8.issuperset(b3));
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
  EXPECT_EQ(r2, (region(::region<long long, 3>(r2))));
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
  vector<box> r12vals;
  r12vals.push_back(b1);
  r12vals.push_back(box(p1, p2));
  region r12(r12vals);
  vector<box> r12boxes = r12;
  EXPECT_EQ(r12vals, r12boxes);
  vector<region> rs;
  rs.push_back(r);
  rs.push_back(r1);
  rs.push_back(r2);
  box b4(p, point(4));
  region r4(b4);
  rs.push_back(r4);
  rs.push_back(r12);
  rs.push_back(r2.difference(r1));
  rs.push_back(r2.symmetric_difference(r12));
  for (std::size_t i = 0; i < rs.size(); ++i) {
    const auto &ri = rs[i];
    auto rgrown = ri.grow(p1);
    auto rshrunk = ri.shrink(p1);
    EXPECT_TRUE(ri.invariant());
    EXPECT_TRUE(rgrown.invariant());
    EXPECT_TRUE(rshrunk.invariant());
    EXPECT_TRUE(rgrown.issuperset(ri));
    if (ri.empty())
      EXPECT_TRUE(rgrown.empty());
    EXPECT_TRUE(ri.issuperset(rshrunk));
    if (!rshrunk.empty())
      EXPECT_TRUE(rshrunk.grow(p1) == ri);
    EXPECT_TRUE(rgrown.shrink(p1) == ri);
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

TEST(RegionCalculus, region2) {
  typedef point<int, 3> point;
  typedef box<int, 3> box;
  typedef region2<int, 3> region2;
  region2 r;
  EXPECT_TRUE(r.invariant());
  EXPECT_TRUE(r.empty());
  point p;
  point p1(1);
  box b;
  box b1(p, p1);
  region2 r0(b);
  EXPECT_TRUE(r0.invariant());
  EXPECT_TRUE(r0.empty());
  region2 r1(b1);
  EXPECT_TRUE(r1.invariant());
  EXPECT_FALSE(r1.empty());
  EXPECT_TRUE(r == r);
  EXPECT_TRUE(r1 == r1);
  EXPECT_FALSE(r != r);
  EXPECT_TRUE(r != r1);
  point p2(2);
  box b2(p, p2);
  region2 r2(b2);
  EXPECT_EQ(r2, region2(::region2<long long, 3>(r2)));
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
  EXPECT_EQ(0, vector<box>(r).size());
  EXPECT_EQ(1, vector<box>(r1).size());
  EXPECT_EQ(1, vector<box>(r2).size());
  auto r21 = r2 - r1;
  vector<box> r21boxes(r21);
  EXPECT_EQ(3, r21boxes.size());
  EXPECT_EQ(box(point(1, 0, 0), point(2, 1, 1)), r21boxes.at(0));
  EXPECT_EQ(box(point(0, 1, 0), point(2, 2, 1)), r21boxes.at(1));
  EXPECT_EQ(box(point(0, 0, 1), point(2, 2, 2)), r21boxes.at(2));
  vector<box> r12vals;
  r12vals.push_back(b1);
  r12vals.push_back(box(p1, p2));
  region2 r12(r12vals);
  vector<box> r12boxes = r12;
  EXPECT_EQ(r12vals, r12boxes);
  vector<region2> rs;
  rs.push_back(r);
  rs.push_back(r1);
  rs.push_back(r2);
  box b4(p, point(4));
  region2 r4(b4);
  rs.push_back(r4);
  rs.push_back(r12);
  rs.push_back(r2.difference(r1));
  rs.push_back(r2.symmetric_difference(r12));
  for (int i = 0; i < 10; ++i) {
    vector<box> bs;
    for (int n = irand(5); n >= 0; --n)
      bs.push_back(box(point(irand(10), irand(10), irand(10)),
                       point(irand(10), irand(10), irand(10))));
    rs.push_back(region2(bs));
  }
  for (std::size_t i = 0; i < rs.size(); ++i) {
    const auto &ri = rs[i];
    EXPECT_TRUE(ri.invariant());
    const auto ri_size = ri.size();
    for (std::size_t j = 0; j < rs.size(); ++j) {
      const auto &rj = rs[j];
      const auto rj_size = rj.size();
      auto rintersection = ri.intersection(rj);
      auto rdifference = ri.difference(rj);
      auto rsetunion = ri.setunion(rj);
      auto rsymmetric_difference = ri.symmetric_difference(rj);
      auto rintersection_size = rintersection.size();
      auto rdifference_size = rdifference.size();
      auto rsetunion_size = rsetunion.size();
      auto rsymmetric_difference_size = rsymmetric_difference.size();
      EXPECT_TRUE(rintersection.invariant());
      EXPECT_TRUE(rdifference.invariant());
      EXPECT_TRUE(rsetunion.invariant());
      EXPECT_TRUE(rsymmetric_difference.invariant());
      EXPECT_TRUE(rintersection_size <= ri_size &&
                  rintersection_size <= rj_size);
      EXPECT_TRUE(rdifference_size <= ri_size &&
                  rdifference_size >= ri_size - rj_size);
      EXPECT_TRUE(rsetunion_size >= ri_size && rsetunion_size >= rj_size &&
                  rsetunion_size <= ri_size + rj_size);
      EXPECT_TRUE(rsymmetric_difference_size <= ri_size + rj_size &&
                  rsymmetric_difference_size >= abs(ri_size - rj_size));
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
      for (int n = 0; n < 10; ++n) {
        int i = irand(10), j = irand(10), k = irand(10);
        point p(i, j, k);
        EXPECT_EQ(ri.contains(p) & rj.contains(p), rintersection.contains(p));
        EXPECT_EQ(ri.contains(p) & !rj.contains(p), rdifference.contains(p));
        EXPECT_EQ(ri.contains(p) | rj.contains(p), rsetunion.contains(p));
        EXPECT_EQ(ri.contains(p) ^ rj.contains(p),
                  rsymmetric_difference.contains(p));
      }
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
  EXPECT_TRUE(equal_to<dpoint>()(p2, dpoint(::dpoint<long long>(p2))));
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
  EXPECT_EQ(b4, dbox(::dbox<long long>(b4)));
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
  EXPECT_EQ(r2, dregion(::dregion<long long>(r2)));
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
  vector<dbox> r12vals;
  r12vals.push_back(b1);
  r12vals.push_back(dbox(p1, p2));
  dregion r12(r12vals);
  vector<dbox> r12boxes = r12;
  EXPECT_EQ(r12vals, r12boxes);
  vector<dregion> rs;
  rs.push_back(r);
  rs.push_back(r1);
  rs.push_back(r2);
  dbox b4(p, dpoint(dim, 4));
  dregion r4(b4);
  rs.push_back(r4);
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
