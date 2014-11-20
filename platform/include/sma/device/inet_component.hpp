#pragma once

#include <sma/device/component.hpp>
#include <sma/net/inet_socket.hpp>


namespace sma
{
  class inet_component : public component
  {
  public:
    virtual ~inet_component() {}

    virtual std::unique_ptr<inet_socket> make_socket() = 0;
  };
}
