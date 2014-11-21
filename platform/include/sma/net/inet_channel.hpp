#pragma once

#include <sma/channel.hpp>

#include <memory>
#include <utility>


namespace sma
{
class inet_socket;

class inet_channel : public channel
{
public:
  inet_channel();
  inet_channel(sink<message const&>* inbox);

  inet_channel(inet_channel&& r)
    : channel(std::forward<inet_channel>(r))
  {}
  inet_channel& operator=(inet_channel&& r)
  {
    channel::operator=(std::forward<inet_channel>(r));
    return *this;
  }

  virtual ~inet_channel() {}

  virtual void add(std::unique_ptr<inet_socket> sock) = 0;
};
}
