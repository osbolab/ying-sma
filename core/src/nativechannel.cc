#include "nativechannel.hh"
#include "nativesocket.hh"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <vector>
#include <memory>
#include <cstdint>
#include <limits>

#include <cerrno>
#include <cstring>


namespace sma
{

NativeChannel::NativeChannel(std::vector<std::unique_ptr<NativeSocket>> sockets)
  : sockets(std::move(sockets))
{
  assert(this->sockets.size() <= MAX_NR_SOCKETS);

  for (auto& sock : this->sockets)
    if (sock->is_blocking(true) != NO_ERROR)
      LOG(ERROR) << "Error setting socket to blocking: " << sock->last_error();
}

std::vector<Socket*> NativeChannel::wait_for_data()
{
  std::vector<Socket*> selected;

  have_data_bitmap = 0;
  fd_set fds;
  FD_ZERO(&fds);
  for (auto& sock : sockets)
    FD_SET(sock->native_socket(), &fds);

  int status = select(sizeof(fds) * std::CHAR_BIT, &fds, NULL, NULL, NULL);
  if (status == -1) {
    LOG(ERROR) << "Selecting socket failed: " << std::strerror(errno);
    return selected;
  }

  if (rc > 0)
    for (auto& sock : sockets)
      if (FD_ISSET(sock->native_socket(), &fds))
        selected.push_back(dynamic_cast<Socket*>(sock.get()));

  return selected;
}

void NativeChannel::broadcast(Message msg)
{
  for (auto& sock : sockets)
    sock->send(msg.cdata(), msg.size(),
}
}

