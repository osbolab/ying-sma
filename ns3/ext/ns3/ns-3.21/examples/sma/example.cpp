#pragma once

//#include <sma/ns3/ns3_socket.hpp>

#include <ns3/core-module.h>
#include <ns3/csma-module.h>
#include <ns3/applications-module.h>
#include <ns3/internet-module.h>

#include <cstdint>
#include <fstream>


NS_LOG_COMPONENT_DEFINE("test_ns3_socket");


int main(int argc, char** argv) {
  ns3::LogComponentEnable("test_ns3_socket", ns3::LOG_LEVEL_INFO);

  NS_LOG_INFO("Send UDP packet from one application to another");

  ns3::NodeContainer nodes;
  nodes.Create(4);

  ns3::CsmaHelper csma;
  csma.SetChannelAttribute("DataRate",
                           ns3::DataRateValue(ns3::DataRate(5242880)));
  csma.SetChannelAttribute("Delay", ns3::TimeValue(ns3::MilliSeconds(2)));
  csma.SetDeviceAttribute("Mtu", ns3::UintegerValue(1400));

  ns3::NetDeviceContainer devices = csma.Install(nodes);

  ns3::InternetStackHelper internet;
  internet.Install(nodes);

  ns3::Ipv4AddressHelper ip;
  ip.SetBase("10.1.1.0", "255.255.255.0");
  ns3::Ipv4InterfaceContainer inet_addrs = ip.Assign(devices);

  ns3::Ipv4Address server_addr;
  server_addr = ns3::Ipv4Address(inet_addrs.GetAddress(0));
  NS_LOG_INFO(" - Server node is at " << server_addr);

  uint16_t port = 9;
  ns3::UdpEchoServerHelper server(port);
  ns3::ApplicationContainer apps = server.Install(nodes.Get(1));
  apps.Start(ns3::Seconds(1.0));
  apps.Stop(ns3::Seconds(10.0));

  size_t packet_size = 1024;
  size_t packet_count = 1;
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
  csma.EnableAsciiAll(ascii.CreateFileStream("udp-echo.tr"));
  csma.EnablePcapAll("udp-echo", false);

  NS_LOG_INFO("Run simulation");
  ns3::Simulator::Run();
  ns3::Simulator::Destroy();
  NS_LOG_INFO("Done");

  return 0;
}
