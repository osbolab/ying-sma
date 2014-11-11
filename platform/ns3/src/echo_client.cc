#include <thread>

#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/address-utils.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"

#include "echo_client.hh"

namespace sma
{
  NS_LOG_COMPONENT_DEFINE("EchoClientApplication");
  NS_OBJECT_ENSURE_REGISTERED(EchoClient);

  ns3::TypeId EchoClient::GetTypeId() {
    static ns3::TypeId tid = ns3::TypeId("barnard::EchoClient")
      .SetParent<Application>()
      .AddConstructor<EchoClient>()
      .AddAttribute("RemotePort", "Destination port of outbound packets.",
        ns3::UintegerValue(9999),
        ns3::MakeUintegerAccessor(&EchoClient::remote_port_),
        ns3::MakeUintegerChecker<uint16_t>())
      .AddAttribute("RemoteAddress", "Destination address of outbound packets.",
        ns3::AddressValue(),
        ns3::MakeAddressAccessor(&EchoClient::remote_addr_),
        ns3::MakeAddressChecker());

    return tid;
  }

  EchoClient::EchoClient() {
    NS_LOG_FUNCTION(this);
  }

  EchoClient::~EchoClient() {
    NS_LOG_FUNCTION(this);
    socket_ = 0;
  }

  void EchoClient::DoDispose() {
    NS_LOG_FUNCTION(this);
    Application::DoDispose();
  }

  void EchoClient::StartApplication() {
    NS_LOG_FUNCTION(this);

    if (socket_ == 0) {
      ns3::TypeId sock_factory_tid = ns3::TypeId::LookupByName("ns3::UdpSocketFactory");
      socket_ = ns3::Socket::CreateSocket(GetNode(), sock_factory_tid);
      socket_->Bind();

      ns3::InetSocketAddress socket_addr = ns3::InetSocketAddress(
        ns3::Ipv4Address::ConvertFrom(remote_addr_), remote_port_);
      socket_->Connect(socket_addr);

      NS_LOG_INFO("[client] Bound UDP socket on " << socket_addr.GetIpv4() << ":" << remote_port_);
    }

    socket_->SetRecvCallback(ns3::MakeCallback(&EchoClient::HandleRead, this));
    packet = ns3::Create<ns3::Packet>(128);


    ns3::Simulator::Schedule(ns3::Seconds(0), &EchoClient::ScheduleTransmit, this);
  }

  void EchoClient::ScheduleTransmit() {
    NS_LOG_FUNCTION(this);

    NS_LOG_INFO("[client] Starting background send thread");


    std::thread sendThread = std::thread([this]() {
      for (int i = 0; i < 5; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        //ns3::Simulator::Schedule(ns3::Seconds(0), &EchoClient::ScheduleTransmit, this);
        Send(packet);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
      }

      NS_LOG_INFO("Shutting down simulation");
      StopApplication();
    });

    NS_LOG_INFO("[client] Background thread running");
    sendThread.detach();

    
  }

  void EchoClient::StopApplication() {
    NS_LOG_FUNCTION(this);

    if (socket_ != 0) {
      socket_->Close();
      socket_->SetRecvCallback(ns3::MakeNullCallback<void, ns3::Ptr<ns3::Socket> >());
    }

    ns3::Simulator::Stop();
  }

  void EchoClient::Send(ns3::Ptr<ns3::Packet> p) {
    NS_LOG_FUNCTION(this);

    socket_->Send(p);
    ++num_sent_;

    NS_LOG_INFO("[client] Sent 128 bytes to " << 
      ns3::Ipv4Address::ConvertFrom(remote_addr_) << ":" << remote_port_);
  }

  void EchoClient::HandleRead(ns3::Ptr<ns3::Socket> socket) {
    NS_LOG_FUNCTION(this << socket);
  }
}