#include "LazyPtr.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <string>

using namespace LazyPtr;

TEST(LazyPtr, pointer) {
  auto p0 = lazy_ptr<int>();
  EXPECT_FALSE(bool(p0));

  auto s1 = std::shared_ptr<int>();
  auto p1 = lazy_ptr<int>(s1);
  EXPECT_FALSE(bool(p1));
  // EXPECT_TRUE(p1.ready());
  EXPECT_EQ(nullptr, p1.get());
  auto p2 = lazy_ptr<int>(std::move(s1));
  EXPECT_FALSE(bool(p2));
  // EXPECT_TRUE(p2.ready());

  auto u3 = std::unique_ptr<int>();
  auto p3 = lazy_ptr<int>(std::move(u3));
  EXPECT_FALSE(bool(p3));
  // EXPECT_TRUE(p3.ready());

  auto p4 = p3;
  EXPECT_FALSE(bool(p4));
  // EXPECT_TRUE(p4.ready());
  auto p5 = std::move(p4);
  EXPECT_FALSE(bool(p5));
  // EXPECT_TRUE(p5.ready());
  EXPECT_FALSE(bool(p3));
  EXPECT_FALSE(bool(p4));

  auto s6 = std::make_shared<int>(1);
  auto p6 = lazy_ptr<int>(s6);
  EXPECT_TRUE(bool(p6));
  // EXPECT_TRUE(p6.ready());
  EXPECT_EQ(1, *p6);
  EXPECT_NE(nullptr, p6.get());
  EXPECT_EQ(s6, p6.shared());
  auto p7 = lazy_ptr<int>(std::move(s6));
  EXPECT_TRUE(bool(p7));
  // EXPECT_TRUE(p7.ready());
  EXPECT_EQ(1, *p7);
  EXPECT_NE(nullptr, p7.get());
  EXPECT_EQ(p6.shared(), p7.shared());

  auto u8 = std::unique_ptr<int>(new int(2));
  auto p8 = lazy_ptr<int>(std::move(u8));
  EXPECT_TRUE(bool(p8));
  // EXPECT_TRUE(p8.ready());
  EXPECT_EQ(2, *p8);
  EXPECT_NE(nullptr, p8.get());
  EXPECT_NE(nullptr, p8.shared());

  auto p9 = p8;
  EXPECT_TRUE(bool(p9));
  // EXPECT_TRUE(p9.ready());
  auto p10 = std::move(p9);
  EXPECT_TRUE(bool(p10));
  // EXPECT_TRUE(p10.ready());
  EXPECT_TRUE(bool(p8));
  EXPECT_FALSE(bool(p9));

  auto p11 = make_ready_lazy<int>(3);
  EXPECT_TRUE(bool(p11));
  // EXPECT_TRUE(p11.ready());
  EXPECT_EQ(3, *p11);
  EXPECT_NE(nullptr, p11.get());
  EXPECT_NE(nullptr, p11.shared());
}

TEST(LazyPtr, function) {
  auto p0 = make_lazy<string>("hello");
  EXPECT_TRUE(bool(p0));
  // EXPECT_FALSE(p0.ready());
  EXPECT_EQ("hello", *p0);
  EXPECT_TRUE(bool(p0));
  // EXPECT_TRUE(p0.ready());

  auto p1 = lazy([] { return string("world"); });
  EXPECT_TRUE(bool(p1));
  // EXPECT_FALSE(p1.ready());

  auto p2 = p1;
  EXPECT_TRUE(bool(p1));
  EXPECT_TRUE(bool(p2));
  // EXPECT_FALSE(p1.ready());
  // EXPECT_FALSE(p2.ready());

  auto p3 = std::move(p2);
  EXPECT_TRUE(bool(p1));
  EXPECT_FALSE(bool(p2));
  EXPECT_TRUE(bool(p3));
  // EXPECT_FALSE(p1.ready());
  // EXPECT_TRUE(p2.ready());
  // EXPECT_FALSE(p3.ready());
  EXPECT_EQ("world", *p1);
  EXPECT_TRUE(bool(p1));
  EXPECT_TRUE(bool(p3));
  // EXPECT_TRUE(p1.ready());
  // EXPECT_TRUE(p3.ready());
  EXPECT_EQ(string("hello").length(), (*p1).length());
  EXPECT_EQ(string("hello").length(), p3->length());
}

#include "src/gtest_main.cc"
