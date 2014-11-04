#include "nativechannel.hh"
#include "nativesocket.hh"

#include "log.hh"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <mutex>
#include <vector>
#include <memory>
#include <cstdint>
#include <climits>

#include <cassert>
#include <cerrno>
#include <cstring>


namespace sma
{

  NativeChannel::NativeChannel(NativeSocket* sock)
    : sockets(1)
  {
    sockets[0] = sock;
    if (sock->is_blocking(true) != NO_ERROR)
      LOG(ERROR) << "Error setting socket to blocking: " << sock->last_error();

  LOG(DEBUG) << "channel of one native socket";
  }

NativeChannel::NativeChannel(std::vector<NativeSocket*> sockets)
  : sockets(std::move(sockets))
{
  for (auto& sock : this->sockets)
    if (sock->is_blocking(true) != NO_ERROR)
      LOG(ERROR) << "Error setting socket to blocking: " << sock->last_error();

  LOG(DEBUG) << "channel of " << this->sockets.size() << " native sockets";
}

std::size_t NativeChannel::wait_for_read(std::uint8_t* dst, std::size_t len)
{
  LOG(DEBUG) << "try to read " << len << " into " << reinterpret_cast<void*>(dst);
  assert(dst);
  assert(len > 0);

  // Constantly take exclusive access to the sockets and try to take one
  std::unique_lock<std::mutex> lock(reader_mutex);
  // If none is ready to read we'll release the lock until it is
  if (readable.empty()) {
    LOG(DEBUG) << "blocking until socket is readable";
    avail.wait(lock, [&] { return !readable.empty(); });
    LOG(DEBUG) << "unblocked!";
  }
  // We have the lock again, and thus ownership of the item in readable
  auto sock = std::move(readable.back());
  readable.pop_back();
  // Now we have a socket so we can unlock for others
  lock.unlock();
  // And wake them if necessary
  if (!readable.empty())
    avail.notify_one();

  assert(socket);
  return sock->recv(dst, len);
}

void NativeChannel::select()
{
  LOG(DEBUG) << "selecting " << sockets.size() << " socket(s)";
  // I don't know who would call this twice but... don't
  std::unique_lock<std::mutex> lock(selecting);

  fd_set fds;
  FD_ZERO(&fds);
  for (auto& sock : sockets)
    FD_SET(sock->native_socket(), &fds);

  int status = ::select(sizeof(fds) * CHAR_BIT, &fds, NULL, NULL, NULL);
  if (status == -1) {
    LOG(ERROR) << "Selecting socket failed: " << std::strerror(errno);
    return;
  }

  {
    // Block readers from potentially seeing an empty list and going to sleep
    // while we're giving them something to read.
    std::unique_lock<std::mutex> lock(reader_mutex);
    if (status > 0)
      for (auto& sock : sockets)
        if (FD_ISSET(sock->native_socket(), &fds))
          readable.push_back(sock);
  }

  if (!readable.empty())
    avail.notify_one();
}
}
