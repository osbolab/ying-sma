#pragma once

#include <sma/nodeinfo.hpp>
#include <sma/component.hpp>
#include <sma/io/log>

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
public:
  Context(NodeInfo node_info, Messenger* msgr, Async* async)
    : node_info(node_info)
    , msgr(msgr)
    , async(async)
  {
    // Create if doesn't exist
    logger = el::Loggers::getLogger(this->node_info.id());
  }
  Context(Context&& r)
    : node_info(std::move(r.node_info))
    , logger(r.logger)
    , msgr(r.msgr)
    , async(r.async)
    , actors(std::move(r.actors))
    , components(std::move(r.components))
  {
    r.msgr = nullptr;
    r.async = nullptr;
  }
  Context& operator=(Context&& r)
  {
    std::swap(node_info, r.node_info);
    logger = r.logger;
    std::swap(msgr, r.msgr);
    std::swap(async, r.async);
    std::swap(actors, r.actors);
    std::swap(components, r.components);
    return *this;
  }

  NodeInfo const* this_node() const;

  void add_component(Component* c);
  template <typename T>
  T* try_get_component() const;

private:
  friend class Actor;

  using log_type = el::Logger;
  log_type* log() const;

  void enter(Actor* actor);
  void leave(Actor* actor);

  template <typename A>
  Actor* get_actor_by_type();

  std::vector<std::pair<std::size_t, Actor*>> actors;
  std::mutex mx;

  log_type* logger;
  NodeInfo node_info;
  Messenger* msgr;
  Async* async;
  std::vector<Component*> components;
};


template <typename T>
Actor* Context::get_actor_by_type()
{
  static_assert(std::is_base_of<Actor, T>::value,
                "Type must derive from Actor");

  std::size_t const hash = typeid(T).hash_code();
  std::lock_guard<std::mutex> lock(mx);
  for (auto& pair : actors)
    if (pair.first == hash)
      return pair.second;
  return nullptr;
}


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
