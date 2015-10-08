#include "SimulationIO.hpp"

#include <gtest/gtest.h>

#include <sstream>

using std::ostringstream;

using namespace SimulationIO;

TEST(ipow, ipow) {
  EXPECT_EQ(1, ipow(0, 0)); // Yes, this needs to be 1
  EXPECT_EQ(0, ipow(0, 1));
  EXPECT_EQ(0, ipow(0, 2));
  EXPECT_EQ(1, ipow(1, 0));
  EXPECT_EQ(1, ipow(1, 1));
  EXPECT_EQ(1, ipow(1, 2));
  EXPECT_EQ(1, ipow(2, 0));
  EXPECT_EQ(2, ipow(2, 1));
  EXPECT_EQ(4, ipow(2, 2));
}

TEST(TensorTypes, Scalar3D) {
  const auto &tt = tensortypes.at("Scalar3D");
  EXPECT_TRUE(tt);
  EXPECT_EQ(3, tt->dimension);
  EXPECT_EQ(0, tt->rank);
  EXPECT_EQ(1, tt->storedcomponents.size());
  EXPECT_TRUE(tt->invariant());
  for (const auto &tc : tt->storedcomponents)
    EXPECT_TRUE(tc->invariant());
  ostringstream buf;
  buf << *tt;
  EXPECT_EQ("TensorType \"Scalar3D\": dim=3 rank=0\n"
            "  TensorComponent \"scalar\": tensortype=\"Scalar3D\" "
            "storedcomponent=0 indices=[]\n",
            buf.str());
}

TEST(TensorTypes, Vector3D) {
  const auto &tt = tensortypes.at("Vector3D");
  EXPECT_TRUE(tt);
  EXPECT_EQ(3, tt->dimension);
  EXPECT_EQ(1, tt->rank);
  EXPECT_EQ(3, tt->storedcomponents.size());
  EXPECT_TRUE(tt->invariant());
  for (const auto &tc : tt->storedcomponents)
    EXPECT_TRUE(tc->invariant());
  ostringstream buf;
  buf << *tt;
  EXPECT_EQ("TensorType \"Vector3D\": dim=3 rank=1\n"
            "  TensorComponent \"0\": tensortype=\"Vector3D\" "
            "storedcomponent=0 indices=[0]\n"
            "  TensorComponent \"1\": tensortype=\"Vector3D\" "
            "storedcomponent=1 indices=[1]\n"
            "  TensorComponent \"2\": tensortype=\"Vector3D\" "
            "storedcomponent=2 indices=[2]\n",
            buf.str());
}

TEST(TensorTypes, SymmetricTensor3D) {
  const auto &tt = tensortypes.at("SymmetricTensor3D");
  EXPECT_TRUE(tt);
  EXPECT_EQ(3, tt->dimension);
  EXPECT_EQ(2, tt->rank);
  EXPECT_EQ(6, tt->storedcomponents.size());
  EXPECT_TRUE(tt->invariant());
  for (const auto &tc : tt->storedcomponents)
    EXPECT_TRUE(tc->invariant());
  ostringstream buf;
  buf << *tt;
  EXPECT_EQ("TensorType \"SymmetricTensor3D\": dim=3 rank=2\n"
            "  TensorComponent \"00\": tensortype=\"SymmetricTensor3D\" "
            "storedcomponent=0 "
            "indices=[0,0]\n"
            "  TensorComponent \"01\": tensortype=\"SymmetricTensor3D\" "
            "storedcomponent=1 "
            "indices=[0,1]\n"
            "  TensorComponent \"02\": tensortype=\"SymmetricTensor3D\" "
            "storedcomponent=2 "
            "indices=[0,2]\n"
            "  TensorComponent \"11\": tensortype=\"SymmetricTensor3D\" "
            "storedcomponent=3 "
            "indices=[1,1]\n"
            "  TensorComponent \"12\": tensortype=\"SymmetricTensor3D\" "
            "storedcomponent=4 "
            "indices=[1,2]\n"
            "  TensorComponent \"22\": tensortype=\"SymmetricTensor3D\" "
            "storedcomponent=5 "
            "indices=[2,2]\n",
            buf.str());
}

TEST(Manifold, create) {
  EXPECT_TRUE(manifolds.empty());
  auto m1 = new Manifold("m1", 3);
  EXPECT_EQ(1, manifolds.size());
  EXPECT_EQ(m1, manifolds.at("m1"));
}

TEST(TangentSpace, create) {
  EXPECT_TRUE(tangentspaces.empty());
  auto s1 = new TangentSpace("s1", 3);
  EXPECT_EQ(1, tangentspaces.size());
  EXPECT_EQ(s1, tangentspaces.at("s1"));
}

TEST(Field, create) {
  EXPECT_TRUE(fields.empty());
  auto f1 = new Field("f1", manifolds.at("m1"), tangentspaces.at("s1"),
                      tensortypes.at("Scalar3D"));
  EXPECT_EQ(1, fields.size());
  EXPECT_EQ(f1, fields.at("f1"));
}

#include "src/gtest_main.cc"
