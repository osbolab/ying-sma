#include <sma/net/inet_addr.hpp>

#include <arpa/inet.h>


namespace sma
{

const inet_addr inet_addr::ANY = inet_addr();
const inet_addr inet_addr::LOOPBACK = inet_addr("127.0.0.1");

}
