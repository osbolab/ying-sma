#pragma once

#include <winsock.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>


// Defined in winsock2; used to close a stream socket for sending but not receiving.
#ifndef SD_SEND
#define SD_SEND 1
#endif

#define BUF_SIZE  8192


struct Client
{
  Client(SOCKET sock, sockaddr_in remote_addr)
    : sock(sock), remote_addr(remote_addr) {}

  bool handle_selection(FD_SET& readable_fds,
                        FD_SET& writable_fds,
                        FD_SET& excepted_fds)
  {
    bool ok = true;

    if (FD_ISSET(sock, &excepted_fds)) {
      std::cerr << "Exception on client at " << std::string(*this) << std::endl;

      FD_CLR(sock, &excepted_fds);
      ok = false;
    }

    if (ok && FD_ISSET(sock, &readable_fds)) {
      ok &= read();
      FD_CLR(sock, &readable_fds);
    }
    if (ok && FD_ISSET(sock, &writable_fds)) {
      ok &= write();
      FD_CLR(sock, &writable_fds);
    }

    return ok;
  }

  bool read()
  {
    if (inbox.size() == BUF_SIZE) {
      std::cerr << "Inbox overrun for client " << std::string(*this) << std::endl;
      return false;
    }

    char buf[BUF_SIZE];
    int read = recv(sock, buf, BUF_SIZE - inbox.size(), 0);
    if (read == 0) {
      std::cout << "* " << std::string(*this) << " went away" << std::endl;
      return false;
    }

    if (read == SOCKET_ERROR) {
      int err;
      int errlen = sizeof(err);
      getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*)&err, &errlen);
      return err == WSAEWOULDBLOCK;
    }

    inbox.insert(inbox.end(), buf, buf + read);
    std::cout << std::string(*this) << ": " << std::string(inbox.begin(), inbox.end());
    if (inbox.back() != '\n')
      std::cout << std::endl;
    inbox.clear();

    return true;
  }

  bool write()
  {
    if (outbox.empty()) {
      std::cerr << "Outbox underrun for client " << std::string(*this) << std::endl;
      return true;
    }

    char buf[BUF_SIZE];
    std::memcpy(buf, &outbox[0], outbox.size());

    int wrote = send(sock, buf, outbox.size(), 0);
    outbox.erase(outbox.begin(), outbox.begin() + wrote);

    return true;
  }

  bool close()
  {
    // Signal to the client that we're finished with the connection
    if (shutdown(sock, SD_SEND) == SOCKET_ERROR)
      return false;

    // Wait for the client to flush its stream
    char buf[BUF_SIZE];
    while (1) {
      // Block until the client closes the TCP connection (indicated by receiving 0 bytes).
      int read = recv(sock, buf, sizeof(buf), 0);
      if (read == SOCKET_ERROR)
        return false;
      if (read == 0)
        break;
    }

    return closesocket(sock) != SOCKET_ERROR;
  }

  explicit operator std::string() const
  {
    std::ostringstream os;
    os << inet_ntoa(remote_addr.sin_addr)
      << ":" << ntohs(remote_addr.sin_port);
    return os.str();
  }

  SOCKET sock;
  sockaddr_in remote_addr;
  std::vector<char> outbox;
  std::vector<char> inbox;
};