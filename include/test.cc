#pragma once

#include <functional>


template<class T>
class Test
{
  T* t;

public:
  Test(std::function<T*()> foo) : t(foo()) {}
};


int main() {
  auto bar = []() { return new char[1]; };

  auto test = Test<char*>(bar);
}