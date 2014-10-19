#include <cassert>
#include <sstream>

#include "gtest/gtest.h"

#include "net/inet_address.hh"
#include "net/SocketAddress.hh"


TEST(ParseAddress, AssertionTrue)
{
  std::string ip = std::string("127.0.0.1");
  std::string ip2 = std::string("128.0.0.2");
  const sma::Address& addr = sma::Ipv4Address(ip);;

  std::ostringstream os;
  os << addr;
  ASSERT_EQ(os.str(), ip);
  ASSERT_NE(os.str(), ip2);
}