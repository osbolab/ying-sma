#pragma once

#ifndef _WINSOCK2API_
  #define WIN32_LEAN_AND_MEAN
  #include <WinSock2.h>
  #include <WS2tcpip.h>
#endif

#ifndef WS2_32_LIB_
  #define WS2_32_LIB_
  #pragma comment(lib, "ws2_32.lib")
#endif

#ifndef ARPA_INET_H_
#define ARPA_INET_H_
#define INET_PTON_H_
namespace sma
{
// WS2tcpip::InetPton is only available starting with Windows Vista and Server 2008
// These are defined in <arpa/inet.h> on BSD and POSIX systems.
int inet_pton(int af, const char* src, void* dst);
const char* inet_ntop(int af, const void* src, char* dst, socklen_t size);
}
#endif