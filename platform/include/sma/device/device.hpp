#pragma once

#include <sma/device/component.hpp>

#include <vector>
#include <memory>


namespace sma
{
  class component;

class device final
{
public:
  device(std::vector<std::unique_ptr<component>> components)
    : components(std::move(components))
  {}

  template <typename T>
  T* try_get_component()
  {
    for (auto& component : components) {
      auto p = dynamic_cast<T*>(component);
      if (p)
        return p;
    }
  }

private:
  std::vector<std::unique_ptr<component>> components;
};
}
