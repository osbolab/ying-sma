#include <sma/context.hpp>
#include <sma/actor.hpp>
#include <sma/nodeinfo.hpp>
#include <sma/component.hpp>

#include <cstdint>
#include <mutex>
#include <utility>


namespace sma
{
NodeInfo const* Context::this_node() const { return &node_info; }
Context::log_type* Context::log() const { return logger; }

void Context::add_component(Component* c)
{
  for (auto& component : components)
    if (typeid(*component) == typeid(*c))
      return;

  components.push_back(c);
}

void Context::enter(Actor* actor)
{
  std::size_t const hash = typeid(*actor).hash_code();
  std::lock_guard<std::mutex> lock(mx);
  for (auto& pair : actors)
    if (pair.first == hash) {
      pair.second = std::move(actor);
      return;
    }
  actors.push_back(std::make_pair(hash, actor));
  logger->trace("context { actors: %v, components: %v }",
                actors.size(),
                components.size());
}

void Context::leave(Actor* actor)
{
  std::lock_guard<std::mutex> lock(mx);
  for (auto it = actors.begin(); it != actors.end(); ++it)
    if (it->second == actor) {
      actors.erase(it);
      return;
    }
}
}
