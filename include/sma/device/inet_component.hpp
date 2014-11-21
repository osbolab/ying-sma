#pragma once

#include <sma/device/component.hpp>


namespace sma
{
  class inet_socket;
  class inet_channel;

  class inet_component : public component
  {
  public:
    virtual ~inet_component() {}

    virtual std::unique_ptr<inet_socket> make_socket() = 0;
    virtual std::unique_ptr<inet_channel> make_channel() = 0;
  };
}
