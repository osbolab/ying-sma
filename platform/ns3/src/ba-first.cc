#include <thread>
#include <iostream>
#include <cstdlib>
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"

#include "echo_server.hh"
#include "echo_client.hh"

NS_LOG_COMPONENT_DEFINE("BarnardFirst");

using namespace ns3;


int main(int argc, char *argv[]) {
  LogComponentEnable("BarnardFirst", LOG_LEVEL_INFO);
  LogComponentEnable("EchoServerApplication", LOG_LEVEL_INFO);
  LogComponentEnable("EchoClientApplication", LOG_LEVEL_FUNCTION);

  NS_LOG_INFO("Send UDP packet from one application to another and echo back");

  NS_LOG_INFO("\nSet realtime mode");
  GlobalValue::Bind("SimulatorImplementationType", StringValue("ns3::RealtimeSimulatorImpl"));

  NS_LOG_INFO("\nhardware\n--------------------");

  NS_LOG_INFO("Create 2 nodes");
  NodeContainer nodes;
  nodes.Create(2);

  NS_LOG_INFO("Create CSMA channel:");
  NS_LOG_INFO(" - 5MBps");
  NS_LOG_INFO(" - 2ms delay");
  NS_LOG_INFO(" - 1400b MTU");

  CsmaHelper csma;
  csma.SetChannelAttribute("DataRate", DataRateValue(DataRate(5242880)));
  csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));
  csma.SetDeviceAttribute("Mtu", UintegerValue(1400));

  NS_LOG_INFO("Add nodes to CSMA channel");
  NetDeviceContainer devices = csma.Install(nodes);

  NS_LOG_INFO("--------------------\n\nsoftware\n--------------------");

  NS_LOG_INFO("Add nodes to Internet stack");
  InternetStackHelper internet;
  internet.Install(nodes);

  NS_LOG_INFO("Assign IPv4 addresses to nodes:");
  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer inetAddresses = ipv4.Assign(devices);

  // The address of the "server" node that echoes packets
  Ipv4Address client_address, server_address;
  client_address = Ipv4Address(inetAddresses.GetAddress(1));
  server_address = Ipv4Address(inetAddresses.GetAddress(0));
  NS_LOG_INFO(" - Client node is at " << client_address);
  NS_LOG_INFO(" - Server node is at " << server_address);

  NS_LOG_INFO("Create Server Application");

  ApplicationContainer apps;

  ObjectFactory server_factory;
  server_factory.SetTypeId(sma::EchoServer::GetTypeId());
  server_factory.Set("Port", UintegerValue(9999));
  Ptr<Application> server_app = server_factory.Create<sma::EchoServer>();
  apps = ApplicationContainer(server_app);
  apps.Start(Seconds(0));
  // Stopped when the simulator stops
  nodes.Get(0)->AddApplication(server_app);

  ObjectFactory client_factory;
  client_factory.SetTypeId(sma::EchoClient::GetTypeId());
  client_factory.Set("RemotePort", UintegerValue(9999));
  client_factory.Set("RemoteAddress", AddressValue(Address(server_address)));
  Ptr<Application> client_app = client_factory.Create<sma::EchoClient>();
  apps = ApplicationContainer(client_app);
  apps.Start(Seconds(1));
  // Stopped after sending all messages and stops the simulator.
  nodes.Get(1)->AddApplication(client_app);

  NS_LOG_INFO("--------------------\n\nSimulation\n--------------------");
  Simulator::Run();
  Simulator::Destroy();
  NS_LOG_INFO("--------------------\n\nDone.");
}
