#include "inetaddress.hh"

#include <arpa/inet.h>


namespace sma
{

const InetAddress InetAddress::ANY = InetAddress();
const InetAddress InetAddress::LOOPBACK = InetAddress("127.0.0.1");

}
