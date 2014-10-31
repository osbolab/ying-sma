#include "inetaddress.hh"
#include "socketaddress.hh"

#include "gtest/gtest.h"

#include <cassert>
#include <sstream>


namespace sma
{

TEST(InetAddress, parse_ip)
{
  std::string ip = std::string("127.0.0.1");
  const Address& addr = InetAddress(ip);
}


TEST(InetAddress, compare)
{
  const Address& addr = InetAddress("127.0.0.1");
  const Address& addr2 = InetAddress("127.0.0.2");

  ASSERT_EQ(InetAddress::LOOPBACK, addr);
  ASSERT_NE(addr, addr2);
}

TEST(InetAddress, print)
{
  const Address& addr = InetAddress("127.0.0.1");

  std::ostringstream os;
  os << addr;
  ASSERT_EQ("127.0.0.1", os.str());
}
}
