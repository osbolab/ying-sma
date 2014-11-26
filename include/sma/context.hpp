#pragma once

#include <sma/component.hpp>
#include <sma/io/log>

#include <vector>
#include <random>

namespace sma
{
class Messenger;
class Async;

class Context final
{
  friend class Actor;

  using prng = std::default_random_engine;

public:
  using prand_value = prng::result_type;

  Context(std::string name, Messenger* msgr, Async* async);
  Context(Context&& r);
  Context& operator=(Context&& r);

  Logger const log() const { return logger; }
  prand_value rand() { return prng_impl(); }

  void add_component(Component* c);
  template <typename T>
  T* try_get_component() const;

private:
  Logger logger;
  Messenger* msgr;
  Async* async;
  std::vector<Component*> components;
  prng prng_impl;
};

template <typename T>
T* Context::try_get_component() const
{
  static_assert(std::is_base_of<Component, T>::value,
                "Type must derive from Component");

  for (auto& c : components) {
    auto t = dynamic_cast<T*>(c);
    if (t)
      return t;
  }
  return nullptr;
}
}
