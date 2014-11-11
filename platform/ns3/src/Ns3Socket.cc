#include "ns3/socket.h"

#include "Ns3Socket.hh"

namespace sma
{

Ns3Socket::Ns3Socket(int domain, int type, int protocol) : AbstractSocket(domain, type, protocol) {
}

Ns3Socket::~Ns3Socket() {
}

int Ns3Socket::bind(const struct sockaddr* addr, socklen_t addrlen) {

}

}