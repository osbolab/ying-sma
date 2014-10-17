#include <vector>
#include <iostream>
#include <cstdlib>
#include <limits>

#include "gtest/gtest.h"

#include "memory/ObjectPool.hh"


typedef std::vector<int> ObjType;

void foo(std::shared_ptr<sma::ObjectPool<ObjType>>& pool)
{
  auto v1 = pool->get();
  v1->push_back(1);
  v1->push_back(1);
}

TEST(Reclaim_Out_of_Scope, AssertionTrue)
{
  auto allocator = []() { return new ObjType; };
  auto deleter = [](ObjType* p) { delete p; };

  auto pool = sma::ObjectPool<ObjType>::create(1, allocator, deleter);
  foo(pool);
  auto v1 = pool->get();
  v1->push_back((*v1)[0] + (*v1)[1]);
  v1->push_back((*v1)[1] + (*v1)[2]);
  v1->push_back((*v1)[2] + (*v1)[3]);

  ASSERT_EQ((*v1)[0], 1);
  ASSERT_EQ((*v1)[1], 1);
  ASSERT_EQ((*v1)[2], 2);
  ASSERT_EQ((*v1)[3], 3);
  ASSERT_EQ((*v1)[4], 5);

  sma::pooled_ptr<ObjType> v2(v1);


  ASSERT_FALSE((*v1).empty());
  const ObjType& rv = *v1;
  const ObjType::const_iterator& cv = v1->cbegin();
}

TEST(Big_One, AssertionTrue)
{
  auto allocator = []() { char* c = new char[1024]; c[1023] = 'A'; return c; };
  auto deleter = [](char* p) { delete [] p; };

  auto pool = sma::ObjectPool<char>::create(1000, allocator, deleter);

  std::vector<sma::pooled_ptr<char>> objs;

  for (int i = 0; i < 1000; ++i) {
    auto o = std::move(pool->get());
    ASSERT_EQ('A', o[1023]);
    o[511] = 'B';
    ASSERT_EQ('B', o[511]);
    const char c = o[255];
    objs.push_back(o);
    if (std::rand() % 10 > 5) objs.pop_back();
  }

  //std::cout << "Press Enter to continue";
  //std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}