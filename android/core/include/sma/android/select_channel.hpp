#pragma once

#include "channel.hpp"

#include "socket.hpp"
#include "nativesocket.hpp"
#include "message.hpp"

#include <mutex>
#include <memory>
#include <cstdlib>
#include <cstdint>
#include <vector>



namespace sma
{

class NativeChannel : public Channel
{
public:
  NativeChannel(NativeSocket* socket);
  NativeChannel(std::vector<NativeSocket*> sockets);

  std::size_t wait_for_read(std::uint8_t* dst, std::size_t len) override;
  // Block until a socket has something to read.
  void select();

private:
  std::mutex selecting;
  std::vector<NativeSocket*> sockets;
  std::vector<NativeSocket*> readable;
};
}
