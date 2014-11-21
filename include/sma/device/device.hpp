#pragma once

#include <sma/device/component.hpp>

#include <vector>
#include <memory>
#include <utility>
#include <type_traits>


namespace sma
{
class device final
{
public:
  device() {}
  device(std::vector<std::unique_ptr<component>> components)
    : components(std::move(components))
  {
  }

  void add_component(std::unique_ptr<component> c)
  {
    if (c)
      components.push_back(std::move(c));
  }

  template <typename T>
  T* try_get();

private:
  std::vector<std::unique_ptr<component>> components;
};

template <typename T>
T* device::try_get()
{
  static_assert(std::is_base_of<component, T>::value,
                "T must be a descendant of component.");
  for (auto& component : components) {
    auto p = dynamic_cast<T*>(component.get());
    if (p)
      return p;
  }
}
}
