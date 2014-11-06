#pragma once

#ifndef _WINSOCK2API_
#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <WS2tcpip.h>
#define TYPEDEF_SOCKET_
#endif

#ifndef WS2_32_LIB_
#define WS2_32_LIB_
#pragma comment(lib, "ws2_32.lib")
#endif

#include <string>

namespace sma
{
#ifndef ARPA_INET_H_
#define ARPA_INET_H_
#define INET_PTON_H_
// WS2tcpip::InetPton is only available starting with Windows Vista and Server 2008
// These are defined in <arpa/inet.h> on BSD and POSIX systems.
extern int inet_pton(int af, const char* src, void* dst);
extern const char* inet_ntop(int af, const void* src, char* dst, socklen_t size);
#endif

extern std::string WSAGetLastErrorMessage(std::string err_message_prefix, int error_code = 0);
}
