#include "net/BsdUdpSocketImpl.hh"

namespace sma
{

int BsdUdpSocketImpl::bind(const Address& address)
{
  return -1;
}

int BsdUdpSocketImpl::close()
{
  return -1;
}

std::unique_ptr<const Packet> BsdUdpSocketImpl::recvFrom(Address& sender)
{
  return nullptr;
}

int BsdUdpSocketImpl::send(std::unique_ptr<const Packet> packet, const Address& recipient)
{
  return -1;
}

}