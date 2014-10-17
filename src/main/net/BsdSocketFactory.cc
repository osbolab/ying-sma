#include "net/BsdSocket.hh"
#include "net/BsdSocketFactory.hh"


namespace sma
{

int BsdSocketFactory::create(Address::Family family, Socket::Type type, Socket::Protocol protocol,
                             std::unique_ptr<Socket>& socket)
{
  switch (family) {
  case Address::IPv4:
  {
    auto ptr = std::unique_ptr<BsdSocket>(new BsdSocket());
    int error = ptr->create(family, type, protocol);
    if (!error) {
      socket = std::move(ptr);
    }
    return error;
  }

  default:
    return EAFNOSUPPORT;
  }
}

}