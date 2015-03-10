#include "wsaerror.hpp"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <IPHlpApi.h>

#include <cstdint>
#include <cstdlib>

#include <iostream>
#include <sstream>

#include <thread>


#define PORT 9997


int main(int argc, char** argv)
{
  using clock = std::chrono::system_clock;
  using millis = std::chrono::milliseconds;

  auto update_period = std::chrono::seconds(5);

  clock::duration const EMPTY = clock::duration(0);

  clock::duration time = EMPTY;
  clock::duration last_time = EMPTY;
  double avg_jitter_ms = 0.0;

  std::cout << "SMA time server (c) 2015" << std::endl;

  WSAData wsa;
  int error;
  if ((error = WSAStartup(MAKEWORD(2, 2), &wsa)) != 0) {
    std::cerr << "WSAStartup failed with error code " << error << std::endl;
    return 1;
  }

  IP_ADAPTER_INFO *adapters_head;
  unsigned long sz_info;

  sz_info = sizeof(IP_ADAPTER_INFO);
  adapters_head = (IP_ADAPTER_INFO*)malloc(sz_info);

  if (GetAdaptersInfo(adapters_head, &sz_info) != ERROR_SUCCESS) {
    free(adapters_head);
    adapters_head = (IP_ADAPTER_INFO*)malloc(sz_info);
  }

  if ((error = GetAdaptersInfo(adapters_head, &sz_info)) != ERROR_SUCCESS) {
    std::cerr << "Error getting adapter info: " << error << std::endl;
    return 1;
  }

  std::cout << "Available interfaces:" << std::endl;
  std::size_t adapter_count = 0;
  for (PIP_ADAPTER_INFO adapter = adapters_head; adapter != nullptr; adapter = adapter->Next) {
    std::cout << " " << adapter_count++ << ". " << adapter->AdapterName << " ("
              << adapter->IpAddressList.IpAddress.String << ")" << std::endl;
  }

  std::size_t sel_idx = adapter_count;
  while (sel_idx >= adapter_count) {
    std::cout << "Select an interface: ";
    std::cin >> sel_idx;
  }

  PIP_ADAPTER_INFO sel_adapter = adapters_head;
  for (std::size_t i = 0; i < sel_idx; ++i)
    sel_adapter = sel_adapter->Next;

  SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock == INVALID_SOCKET) {
    std::cerr << WSAGetLastErrorMessage("Error creating socket") << std::endl;
    WSACleanup();
    return 1;
  }

  sockaddr_in bind_addr;
  bind_addr.sin_family = AF_INET;
  if (inet_pton(AF_INET, sel_adapter->IpAddressList.IpAddress.String, &bind_addr.sin_addr) == -1) {
    std::cerr << WSAGetLastErrorMessage("Interface does not have a valid IPv4 address") << std::endl;
    WSACleanup();
    return 1;
  }
  if (bind(sock, (sockaddr*)&bind_addr, sizeof(sockaddr_in)) == SOCKET_ERROR) {
    std::cerr << WSAGetLastErrorMessage("Error binding to interface") << std::endl;
    WSACleanup();
    return 1;
  }

  int so_broadcast = 1;
  if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char const*) &so_broadcast, sizeof(so_broadcast)) == -1) {
    std::cerr << WSAGetLastErrorMessage("Error setting broadcast mode") << std::endl;
    WSACleanup();
    return 1;
  }

  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.S_un.S_addr = htonl(INADDR_BROADCAST);
  addr.sin_port = htons(PORT);

  std::cout << "\nBroadcasting to the network at " << sel_adapter->IpAddressList.IpAddress.String << ":" << PORT
            << "\n--------------------------------------" << std::endl;

  while (1) {
    if (time != EMPTY)
      last_time = time;

    time = clock::now().time_since_epoch();

    if (last_time != EMPTY) {
      double const jitter_ms = double(std::chrono::duration_cast<millis>((time - last_time) - update_period).count());
      if (avg_jitter_ms == 0.0)
        avg_jitter_ms = jitter_ms;
      else {
        avg_jitter_ms += jitter_ms;
        avg_jitter_ms /= 2.0;
      }
    }

    std::uint64_t time_ms = std::chrono::duration_cast<millis>(time).count();

    char packet[8] = { 
      char(time_ms >> 56), 
      char(time_ms >> 48), 
      char(time_ms >> 40), 
      char(time_ms >> 32), 
      char(time_ms >> 24), 
      char(time_ms >> 16), 
      char(time_ms >> 8), 
      char(time_ms)
    };

    std::size_t const sz_packet = sizeof(packet);

    int sent = sendto(sock, packet, sz_packet, 0, (sockaddr*)&addr, sizeof(addr));
    if (sent == -1)
      std::cout << "[FAIL] ";
    else
      std::cout << "[OK] ";

    std::cout << "time: " << time_ms << " ms (avg " << std::round(avg_jitter_ms) << " ms jitter)" << std::endl;

    std::this_thread::sleep_for(update_period);
  }

  WSACleanup();

  return 0;
}