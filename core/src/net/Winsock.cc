#ifdef WIN32

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "net/Winsock.hh"

#include <iostream>
#include <algorithm>
#include <sstream>
#include <string>


namespace sma
{

#ifdef INET_PTON_H_

/**
 * inet_pton and inet_ntop implementations from Petar Korponaic on 28/12/13:
 *    http://stackoverflow.com/a/20817001
 * Retrieved 16/10/14 and used without permission.
 */

int inet_pton(int af, const char* src, void* dst)
{
  struct sockaddr_storage ss;
  int size = sizeof(ss);
  char src_copy[INET6_ADDRSTRLEN + 1];

  memset(&ss, 0, sizeof(ss));
  /* stupid non-const API */
  strncpy_s(src_copy, src, INET6_ADDRSTRLEN + 1);
  src_copy[INET6_ADDRSTRLEN] = 0;

  if (WSAStringToAddress(src_copy, af, NULL, (struct sockaddr*)&ss, &size) == 0) {
    switch (af) {
      case AF_INET:
        *(struct in_addr*)dst = ((struct sockaddr_in*)&ss)->sin_addr;
        return 1;
      case AF_INET6:
        *(struct in6_addr*)dst = ((struct sockaddr_in6*)&ss)->sin6_addr;
        return 1;
    }
  }
  return 0;
}

const char* inet_ntop(int af, const void* src, char* dst, socklen_t size)
{
  struct sockaddr_storage ss;
  unsigned long s = size;

  memset(&ss, 0, sizeof(ss));
  ss.ss_family = af;

  switch (af) {
    case AF_INET:
      ((struct sockaddr_in*)&ss)->sin_addr = *(struct in_addr*)src;
      break;
    case AF_INET6:
      ((struct sockaddr_in6*)&ss)->sin6_addr = *(struct in6_addr*)src;
      break;
    default:
      return NULL;
  }
  /* can't directly use &size because of strict aliasing rules */
  return (WSAAddressToString((struct sockaddr*)&ss, sizeof(ss), NULL, dst, &s) == 0) ?
         dst : NULL;
}

#endif

/*
 * From http://tangentsoft.net/
 * Retrieved 10/22/14 and used without permission.
 */
// List of Winsock error constants mapped to an interpretation string.
// Note that this list must remain sorted by the error constants'
// values, because we do a binary search on the list when looking up
// items.
static struct WSAErrorEntry {
  int nID;
  const char* err_message;

  WSAErrorEntry(int id, const char* pc = 0) :
    nID(id),
    err_message(pc)
  {
  }

  bool operator<(const WSAErrorEntry& rhs) const
  {
    return nID < rhs.nID;
  }
} gaErrorList[] = {
  WSAErrorEntry(0,                  "No error"),
  WSAErrorEntry(WSAEINTR,           "Interrupted system call"),
  WSAErrorEntry(WSAEBADF,           "Bad file number"),
  WSAErrorEntry(WSAEACCES,          "Permission denied"),
  WSAErrorEntry(WSAEFAULT,          "Bad address"),
  WSAErrorEntry(WSAEINVAL,          "Invalid argument"),
  WSAErrorEntry(WSAEMFILE,          "Too many open sockets"),
  WSAErrorEntry(WSAEWOULDBLOCK,     "Operation would block"),
  WSAErrorEntry(WSAEINPROGRESS,     "Operation now in progress"),
  WSAErrorEntry(WSAEALREADY,        "Operation already in progress"),
  WSAErrorEntry(WSAENOTSOCK,        "Socket operation on non-socket"),
  WSAErrorEntry(WSAEDESTADDRREQ,    "Destination address required"),
  WSAErrorEntry(WSAEMSGSIZE,        "Message too long"),
  WSAErrorEntry(WSAEPROTOTYPE,      "Protocol wrong type for socket"),
  WSAErrorEntry(WSAENOPROTOOPT,     "Bad protocol option"),
  WSAErrorEntry(WSAEPROTONOSUPPORT, "Protocol not supported"),
  WSAErrorEntry(WSAESOCKTNOSUPPORT, "Socket type not supported"),
  WSAErrorEntry(WSAEOPNOTSUPP,      "Operation not supported on socket"),
  WSAErrorEntry(WSAEPFNOSUPPORT,    "Protocol family not supported"),
  WSAErrorEntry(WSAEAFNOSUPPORT,    "Address family not supported"),
  WSAErrorEntry(WSAEADDRINUSE,      "Address already in use"),
  WSAErrorEntry(WSAEADDRNOTAVAIL,   "Can't assign requested address"),
  WSAErrorEntry(WSAENETDOWN,        "Network is down"),
  WSAErrorEntry(WSAENETUNREACH,     "Network is unreachable"),
  WSAErrorEntry(WSAENETRESET,       "Net connection reset"),
  WSAErrorEntry(WSAECONNABORTED,    "Software caused connection abort"),
  WSAErrorEntry(WSAECONNRESET,      "Connection reset by peer"),
  WSAErrorEntry(WSAENOBUFS,         "No buffer space available"),
  WSAErrorEntry(WSAEISCONN,         "Socket is already connected"),
  WSAErrorEntry(WSAENOTCONN,        "Socket is not connected"),
  WSAErrorEntry(WSAESHUTDOWN,       "Can't send after socket shutdown"),
  WSAErrorEntry(WSAETOOMANYREFS,    "Too many references, can't splice"),
  WSAErrorEntry(WSAETIMEDOUT,       "Connection timed out"),
  WSAErrorEntry(WSAECONNREFUSED,    "Connection refused"),
  WSAErrorEntry(WSAELOOP,           "Too many levels of symbolic links"),
  WSAErrorEntry(WSAENAMETOOLONG,    "File name too long"),
  WSAErrorEntry(WSAEHOSTDOWN,       "Host is down"),
  WSAErrorEntry(WSAEHOSTUNREACH,    "No route to host"),
  WSAErrorEntry(WSAENOTEMPTY,       "Directory not empty"),
  WSAErrorEntry(WSAEPROCLIM,        "Too many processes"),
  WSAErrorEntry(WSAEUSERS,          "Too many users"),
  WSAErrorEntry(WSAEDQUOT,          "Disc quota exceeded"),
  WSAErrorEntry(WSAESTALE,          "Stale NFS file handle"),
  WSAErrorEntry(WSAEREMOTE,         "Too many levels of remote in path"),
  WSAErrorEntry(WSASYSNOTREADY,     "Network system is unavailable"),
  WSAErrorEntry(WSAVERNOTSUPPORTED, "Winsock version out of range"),
  WSAErrorEntry(WSANOTINITIALISED,  "WSAStartup not yet called"),
  WSAErrorEntry(WSAEDISCON,         "Graceful shutdown in progress"),
  WSAErrorEntry(WSAHOST_NOT_FOUND,  "Host not found"),
  WSAErrorEntry(WSANO_DATA,         "No host data of that type was found")
};
const int wsa_nr_errmsgs = sizeof(gaErrorList) / sizeof(WSAErrorEntry);


//// WSAGetLastErrorMessage ////////////////////////////////////////////
// A function similar in spirit to Unix's perror() that tacks a canned
// interpretation of the value of WSAGetLastError() onto the end of a
// passed string, separated by a ": ".  Generally, you should implement
// smarter error handling than this, but for default cases and simple
// programs, this function is sufficient.
//
// This function returns a pointer to an internal static buffer, so you
// must copy the data from this function before you call it again.  It
// follows that this function is also not thread-safe.

std::string WSAGetLastErrorMessage(std::string err_message_prefix, int error_code /* = 0 */)
{
  // Build basic error string
  std::stringstream outs;
  outs << err_message_prefix << ": ";

  // Tack appropriate canned message onto end of supplied message
  // prefix. Note that we do a binary search here: gaErrorList must be
  // sorted by the error constant's value.
  WSAErrorEntry* pEnd = gaErrorList + wsa_nr_errmsgs;
  WSAErrorEntry Target(error_code ? error_code : WSAGetLastError());
  WSAErrorEntry* it = std::lower_bound(gaErrorList, pEnd, Target);
  if ((it != pEnd) && (it->nID == Target.nID)) {
    outs << it->err_message;
  } else {
    // Didn't find error in list, so make up a generic one
    outs << "unknown error";
  }
  outs << " (" << Target.nID << ")";

  // Finish error message off and return it.
  outs << std::ends;
  return outs.str();
}

}

#endif
