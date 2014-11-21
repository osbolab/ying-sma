#pragma once

#include <sma/component.hpp>


namespace sma
{
  class InetSocket;
  class InetChannel;

  class InetComponent : public Component
  {
  public:
    virtual ~InetComponent() {}

    virtual std::unique_ptr<InetSocket> make_socket() = 0;
    virtual std::unique_ptr<InetChannel> make_channel() = 0;
  };
}
