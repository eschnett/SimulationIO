#include "SimulationIO.hpp"

#include <gtest/gtest.h>

using namespace SimulationIO;

TEST(TensorTypes, Scalar3D) {
  EXPECT_EQ(3, Scalar3D.dimension);
  EXPECT_EQ(0, Scalar3D.rank);
  EXPECT_TRUE(Scalar3D.invariant());
  for (const auto &tc : Scalar3D.storedcomponents)
    EXPECT_TRUE(tc->invariant());
}

#include "src/gtest_main.cc"
