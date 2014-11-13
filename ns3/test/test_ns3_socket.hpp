#pragma once

#include <sma/ns3/ns3_socket.hpp>
#include <sma/log.hpp>

#include <ns3/core-module.h>
#include <ns3/csma-module.h>
#include <ns3/applications-module.h>
#include <ns3/internet-module.h>

#include <cstdint>
#include <fstream>


namespace sma
{
TEST(ns3_socket, bind)
{
  const std::size_t nnodes = 2;

  LOG(DEBUG) << "Create CSMA channel";
  LOG(DEBUG) << " - 5MBps\n - 2ms delay\n - 1400b MTU";

  ns3::CsmaHelper csma;
  csma.SetChannelAttribute("DataRate",
                           ns3::DataRateValue(ns3::DataRate(5242880)));
  csma.SetChannelAttribute("Delay", ns3::TimeValue(ns3::MilliSeconds(2)));
  csma.SetDeviceAttribute("Mtu", ns3::UintegerValue(1400));

  LOG(DEBUG) << "Create " << nnodes << " NS3 nodes";
  ns3::NodeContainer nodes;
  nodes.Create(nnodes);

  LOG(DEBUG) << "Attach nodes to channel";
  ns3::NetDeviceContainer devices = csma.Install(nodes);

  LOG(DEBUG) << "Create Internet and assign IP addresses to devices";
  ns3::InternetStackHelper internet;
  internet.Install(nodes);

  ns3::Ipv4AddressHelper ip;
  ip.SetBase("10.1.1.0", "255.255.255.0");
  ns3::Ipv4InterfaceContainer inet_addrs = ip.Assign(devices);

  LOG(DEBUG) << "Install SMA application in nodes";
  ns3::ApplicationContainer apps;

  // NS3 gives us some dependency injection when creating applications.
  // The application in this case is an NS3 wrapper around some executable
  // component so NS3 can drive it; that component is our SMA application.
  // NS3 drives the application by supplying timed signals to the scheduler
  // and packets to the sockets; the sockets in turn call up through the
  // channels to the messengers which deliver messages to the high-level
  // application.
  ns3::ObjectFactory sma_factory;
  sma_factory.SetTypeId(sma_app::GetTypeId());
  sma_factory.Set("Port", 9999);

  // We can keep using that injection template to spawn applications and
  // attach them to nodes.
  for (std::size_t i = 0; i < nnodes; ++i) {
    container_app = sma_factory.Create<sma_app>();
    apps = ns3::ApplicationContainer(container_app);
    apps.Start(ns3::Seconds(0));
    nodes.Get(i)->AddApplication(container_app);
  }

  std::size_t packet_size = 1024;
  std::size_t packet_count = 5;
  ns3::Time packet_interval = ns3::Seconds(1.);

  ns3::UdpEchoClientHelper client(server_addr, port);
  client.SetAttribute("MaxPackets", ns3::UintegerValue(packet_count));
  client.SetAttribute("Interval", ns3::TimeValue(packet_interval));
  client.SetAttribute("PacketSize", ns3::UintegerValue(packet_size));
  apps = client.Install(nodes.Get(0));
  apps.Start(ns3::Seconds(2.0));
  apps.Stop(ns3::Seconds(10.0));

  client.SetFill(apps.Get(0), "Hello, world!");

  ns3::AsciiTraceHelper ascii;
  csma.EnablePcapAll("udp-echo", false);

  LOG(DEBUG) << "Simulating 10 seconds";
  ns3::Simulator::Run();
  ns3::Simulator::Destroy();
  LOG(DEBUG) << "done.";
}
}
