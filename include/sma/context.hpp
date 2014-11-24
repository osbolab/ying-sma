#pragma once

#include <sma/component.hpp>

#include <utility>
#include <vector>
#include <mutex>
#include <typeinfo>


namespace sma
{
class Actor;
class Messenger;
class Async;

class Context final
{
  friend class Actor;

public:
  Context(Messenger* msgr, Async* async)
    : msgr(msgr)
    , async(async)
  {
  }
  Context(Context&& r)
    : msgr(r.msgr)
    , async(r.async)
    , actors(std::move(r.actors))
    , components(std::move(r.components))
  {
    r.msgr = nullptr;
    r.async = nullptr;
  }
  Context& operator=(Context&& r)
  {
    std::swap(msgr, r.msgr);
    std::swap(async, r.async);
    std::swap(actors, r.actors);
    std::swap(components, r.components);
    return *this;
  }

  template <typename T>
  T* try_get_component();

private:
  void enter(Actor* actor);
  void leave(Actor* actor);

  template <typename A>
  Actor* get_actor_by_type();

  std::vector<std::pair<std::size_t, Actor*>> actors;
  std::mutex mx;

  Messenger* msgr;
  Async* async;
  std::vector<Component*> components;
};

template <typename A>
Actor* Context::get_actor_by_type()
{
  static_assert(std::is_base_of<Actor, A>::value,
                "Type must derive from Actor");

  std::size_t const hash = typeid(A).hash_code();
  std::lock_guard<std::mutex> lock(mx);
  for (auto& pair : actors)
    if (pair.first == hash)
      return pair.second;
  return nullptr;
}

template <typename T>
T* Context::try_get_component()
{
  static_assert(std::is_base_of<Component, T>::value,
                "Type must derive from Component");

  for (auto& c : components) {
    if (typeid(*c) == typeid(T))
      return static_cast<T*>(c);
  }
  return nullptr;
}
}
