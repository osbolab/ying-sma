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

#include "echo_server.hh"

namespace sma 
{
  NS_LOG_COMPONENT_DEFINE("EchoServerApplication");
  NS_OBJECT_ENSURE_REGISTERED(EchoServer);

  ns3::TypeId EchoServer::GetTypeId() {
    static ns3::TypeId tid = ns3::TypeId("sma::EchoServer")
      .SetParent<Application>()
      .AddConstructor<EchoServer>()
      .AddAttribute("Port", "Port on which to listen for incoming packets.",
      ns3::UintegerValue(9999),
      ns3::MakeUintegerAccessor(&EchoServer::port_),
      ns3::MakeUintegerChecker<uint16_t>());

    return tid;
  }

  EchoServer::EchoServer() {
    NS_LOG_FUNCTION(this);
  }

  EchoServer::~EchoServer() {
    NS_LOG_FUNCTION(this);
    socket_ = 0;
  }

  void EchoServer::DoDispose() {
    NS_LOG_FUNCTION(this);
    Application::DoDispose();
  }

  void EchoServer::StartApplication() {
    NS_LOG_FUNCTION(this);

    if (socket_ == 0) {
      ns3::TypeId sock_factory_tid = ns3::TypeId::LookupByName("ns3::UdpSocketFactory");
      socket_ = ns3::Socket::CreateSocket(GetNode(), sock_factory_tid);
      ns3::InetSocketAddress socket_addr = ns3::InetSocketAddress(ns3::Ipv4Address::GetAny(), port_);
      socket_->Bind(socket_addr);
      
      NS_LOG_INFO("[server] Bound UDP socket on " << socket_addr.GetIpv4() << ":" << port_);
    }

    socket_->SetRecvCallback(ns3::MakeCallback(&EchoServer::HandleRead, this));
  }

  void EchoServer::StopApplication() {
    NS_LOG_FUNCTION(this);

    if (socket_ != 0) {
      socket_->Close();
      socket_->SetRecvCallback(ns3::MakeNullCallback<void, ns3::Ptr<ns3::Socket> >());
    }
  }

  void EchoServer::HandleRead(ns3::Ptr<ns3::Socket> socket) {
    NS_LOG_FUNCTION(this << socket);

    ns3::Ptr<ns3::Packet> packet;
    ns3::Address source_addr;
    
    while ((packet = socket_->RecvFrom(source_addr))) {
      ns3::InetSocketAddress addr = ns3::InetSocketAddress::ConvertFrom(source_addr);

      NS_LOG_INFO("[server] Received " << packet->GetSize() << " bytes from " <<
        addr.GetIpv4() << ":" << addr.GetPort());
    }
  }
}