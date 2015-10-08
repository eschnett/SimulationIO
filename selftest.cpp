#include "SimulationIO.hpp"

#include <gtest/gtest.h>

using namespace SimulationIO;

TEST(TensorTypes, Scalar3D) {
  const auto &tt = Scalar3D;
  EXPECT_EQ(3, tt.dimension);
  EXPECT_EQ(0, tt.rank);
  EXPECT_EQ(1, tt.storedcomponents.size());
  EXPECT_TRUE(tt.invariant());
  for (const auto &tc : tt.storedcomponents)
    EXPECT_TRUE(tc->invariant());
}

TEST(TensorTypes, Vector3D) {
  const auto &tt = Vector3D;
  EXPECT_EQ(3, tt.dimension);
  EXPECT_EQ(1, tt.rank);
  EXPECT_EQ(3, tt.storedcomponents.size());
  EXPECT_TRUE(tt.invariant());
  for (const auto &tc : tt.storedcomponents)
    EXPECT_TRUE(tc->invariant());
}

TEST(TensorTypes, SymmetricTensor3D) {
  const auto &tt = SymmetricTensor3D;
  EXPECT_EQ(3, tt.dimension);
  EXPECT_EQ(2, tt.rank);
  EXPECT_EQ(6, tt.storedcomponents.size());
  EXPECT_TRUE(tt.invariant());
  for (const auto &tc : tt.storedcomponents)
    EXPECT_TRUE(tc->invariant());
}

#include "src/gtest_main.cc"
