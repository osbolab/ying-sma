#include "wsaerror.hpp"

#include <winsock.h>

#include <iostream>
#include <vector>
#include <string>
#include <sstream>

// Defined in winsock2; used to close a stream socket for sending but not receiving.
#ifndef SD_SEND
  #define SD_SEND 1
#endif

#define BUF_SIZE  8192
#define PORT      9998


struct Client {
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


std::vector<Client> clients;


// Create a socket and start listening on any address at the given port.
SOCKET listen(int const port)
{
  SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock != INVALID_SOCKET) {
    sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.S_un.S_addr = INADDR_ANY;
    sin.sin_port = htons(port);
    if (bind(sock, (sockaddr*)&sin, sizeof(sockaddr_in)) != SOCKET_ERROR) {
      listen(sock, SOMAXCONN);
      return sock;
    } else {
      std::cerr << WSAGetLastErrorMessage("Error binding listen socket") << std::endl;
    }
  } else {
    std::cerr << WSAGetLastErrorMessage("Error creating listen socket") << std::endl;
  }

  return INVALID_SOCKET;
}

// Populate the sets of sockets selectable for reading, writing, or error detection.
void init_fds(fd_set& readable, 
              fd_set& writable, 
              fd_set& excepted, 
              SOCKET sock = INVALID_SOCKET)
{
  FD_ZERO(&readable);
  FD_ZERO(&writable);
  FD_ZERO(&excepted);

  // Select the listener for reading and errors
  if (sock != INVALID_SOCKET) {
    FD_SET(sock, &readable);
    FD_SET(sock, &excepted);
  }

  for (auto& client : clients) {
    // Select client for reading if its incoming buffer is not full
    if (client.inbox.size() < BUF_SIZE)
      FD_SET(client.sock, &readable);
    // Select client for writing if its outgoing buffer is not empty
    if (!client.outbox.empty())
      FD_SET(client.sock, &writable);
    // Select client for errors
    FD_SET(client.sock, &excepted);
  }
}

bool accept_client(SOCKET const sock)
{
  sockaddr_in sin_remote;
  int sz_addr = sizeof(sin_remote);

  SOCKET client_sock = accept(sock, (sockaddr*)&sin_remote, &sz_addr);
  if (client_sock != INVALID_SOCKET) {
    unsigned long so_nonblocking = 1;
    ioctlsocket(client_sock, FIONBIO, &so_nonblocking);

    clients.emplace_back(client_sock, sin_remote);

    std::cout << "* " << std::string(clients.back()) << " joined" << std::endl;

    if (clients.size() == 64)
      std::cout << "Warning: more than 63 clients may not be reliable on some platforms." 
                << std::endl;

    return true;

  } else {
    std::cerr << "Error accepting client connection" << std::endl;
  }

  return false;
}

// Spin forever, blocking each iteration on selecting a socket for reading, 
// writing, or error detection.
void listen_loop(SOCKET const sock)
{
  std::cout << "Accepting connections on port " << PORT << std::endl;

  while (1) {
    fd_set readable_fds, writable_fds, excepted_fds;
    init_fds(readable_fds, writable_fds, excepted_fds, sock);

    if (select(0, &readable_fds, &writable_fds, &excepted_fds, 0) > 0) {
      if (FD_ISSET(sock, &readable_fds)) {
        accept_client(sock);
      } else {
        auto it = clients.begin();
        while (it != clients.end()) {
          if (it->handle_selection(readable_fds, writable_fds, excepted_fds))
            ++it;
          else {
            it->close();
            it = clients.erase(it);
          }
        }
      }
    } 
  }
}

int main(int argc, char** argv)
{
  std::cout << "SMA log server (c) 2015" << std::endl;

  WSAData wsa;
  int error;
  if ((error = WSAStartup(MAKEWORD(1, 1), &wsa)) != 0) {
    std::cerr << "WSAStartup failed with error code " << error << std::endl;
    return 1;
  }

  SOCKET const listener = listen(PORT);
  if (listener == INVALID_SOCKET) {
    std::cerr << "Couldn't create listener" << std::endl;
    return 1;
  }

  while (1) {
    listen_loop(listener);
    std::cout << "Restarting log server..." << std::endl;
  }

  WSACleanup();

  return 0;
}