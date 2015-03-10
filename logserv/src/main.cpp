#include "wsaerror.hpp"

#include "client.hpp"

#include <winsock.h>

#include <iostream>
#include <vector>
#include <string>


#define PORT 9996


std::vector<Client> clients;


// Create a socket and start listening on any address at the given port.
SOCKET listen(int const port)
{
  SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
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

// Allocate a new socket and tx/rx buffers for an incoming client connection.
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
  if ((error = WSAStartup(MAKEWORD(2, 2), &wsa)) != 0) {
    std::cerr << "WSAStartup failed with error code " << error << std::endl;
    return 1;
  }

  SOCKET const listener = listen(PORT);
  if (listener == INVALID_SOCKET) {
    std::cerr << "Couldn't create listener" << std::endl;
    return 1;
  }

  out_file = std::ofstream("log.txt");

  while (1) {
    listen_loop(listener);
    std::cout << "Restarting log server..." << std::endl;
  }

  out_file.close();

  WSACleanup();

  return 0;
}