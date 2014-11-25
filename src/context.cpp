#include <sma/context.hpp>
#include <sma/component.hpp>

namespace sma
{
Context::Context(std::string name, Messenger* msgr, Async* async)
  : logger(name)
  , msgr(msgr)
  , async(async)
{
}
Context::Context(Context&& r)
  : logger(r.logger)
  , msgr(r.msgr)
  , async(r.async)
  , components(std::move(r.components))
{
  r.msgr = nullptr;
  r.async = nullptr;
}

Context& Context::operator=(Context&& r)
{
  logger = r.logger;
  std::swap(msgr, r.msgr);
  std::swap(async, r.async);
  std::swap(components, r.components);
  return *this;
}

void Context::add_component(Component* c)
{
  for (auto& component : components)
    if (typeid(*component) == typeid(*c))
      return;

  components.push_back(c);
}
}
