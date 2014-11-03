#pragma once

#include "socket.hh"
#include "nativesocket.hh"
#include "message.hh"

#include <memory>
#include <cstdlib>
#include <cstdint>
#include <vector>



namespace sma
{

class NativeChannel final
{
public:
  NativeChannel(std::vector<std::unique_ptr<NativeSocket>> sockets);

  // Blocks until some sockets have data ready to read.
  std::vector<Socket*> wait_for_data();
  // Blocks until the message is sent to every socket in the channel.
  void broadcast(Message msg);

private:
  std::uint32_t have_data_bitmap;
  static const std::size_t MAX_NR_SOCKETS{(sizeof have_data_bitmap)
                                          * std::CHAR_BIT};
  std::vector<std::unique_ptr<NativeSocket>> sockets;
};
}
