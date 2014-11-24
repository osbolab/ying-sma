#include <sma/context.hpp>
#include <sma/actor.hpp>
#include <sma/nodeinfo.hpp>

#include <cstdint>
#include <mutex>
#include <utility>


namespace sma
{
NodeInfo* Context::this_node() { return &node_info; }

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
