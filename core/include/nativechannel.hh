#pragma once

#include "channel.hh"

#include "socket.hh"
#include "nativesocket.hh"
#include "message.hh"

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
  NativeChannel(std::vector<std::unique_ptr<NativeSocket>> sockets);

  std::size_t wait_for_read(std::uint8_t* dst, std::size_t len) override;
  // Block until a socket has something to read.
  void select();

private:
  std::mutex selecting;
  std::vector<std::unique_ptr<NativeSocket>> sockets;
  std::vector<NativeSocket*> readable;
};
}
