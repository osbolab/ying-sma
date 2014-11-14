#include "simple_launcher.hpp"

#include <sma/ns3/ns3_socket.hpp>
#include <sma/ns3/container_app.hpp>
#include <sma/log.hpp>

#include <ns3/core-module.h>
#include <ns3/csma-module.h>
#include <ns3/applications-module.h>
#include <ns3/internet-module.h>

#include <cstdint>
#include <fstream>


namespace sma
{
  const std::size_t nnodes = 2;

  LOG(DEBUG) << "Create CSMA channel (5MBps - 2ms delay - 1400b MTU)";

  ns3::CsmaHelper csma;
  csma.SetChannelAttribute("DataRate",
                           ns3::DataRateValue(ns3::DataRate(5242880)));
  csma.SetChannelAttribute("Delay", ns3::TimeValue(ns3::MilliSeconds(2)));
  csma.SetDeviceAttribute("Mtu", ns3::UintegerValue(1400));

  LOG(DEBUG) << "Create " << nnodes << " simulator nodes and attach to channel";
  ns3::NodeContainer nodes;
  nodes.Create(nnodes);
  ns3::NetDeviceContainer devices = csma.Install(nodes);

  LOG(DEBUG) << "Create Internet and assign IP addresses to devices";
  ns3::InternetStackHelper internet;
  internet.Install(nodes);

  ns3::Ipv4AddressHelper ip;
  ip.SetBase("10.1.1.0", "255.255.255.0");
  ns3::Ipv4InterfaceContainer inet_addrs = ip.Assign(devices);

  // ------------------------- SMA STUFF --------------------------------
  // NS3 gives us some dependency injection when creating applications.
  // The application in this case is an NS3 wrapper around some executable
  // component so NS3 can drive it; that component is our SMA application.
  // NS3 drives the application by supplying timed signals to the scheduler
  // and packets to the sockets; the sockets in turn call up through the
  // channels to the messengers which deliver messages to the high-level
  // application.
  LOG(DEBUG) << "Install SMA application instances in simulator nodes";
  ns3::ObjectFactory sma_factory;
  sma_factory.SetTypeId(container_app::TypeId());
  sma_factory.Set("Port", ns3::UintegerValue(9999));

  // We can keep using that injection template to spawn applications and
  // attach them to nodes.
  for (std::size_t i = 0; i < nnodes; ++i) {
    sma_factory.Set("ID", ns3::UintegerValue(i));
    auto app = sma_factory.Create<container_app>();
    auto apps = ns3::ApplicationContainer(app);
    apps.Start(ns3::Seconds(0));
    nodes.Get(i)->AddApplication(app);
  }
  // ^^^^^^^^^^^^^^^^^^^^^^^^^ SMA STUFF ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

  LOG(DEBUG) << "Will output pcap data for traffic analysis";
  ns3::AsciiTraceHelper ascii;
  csma.EnablePcapAll("test_ns3_socket", false);

  LOG(DEBUG) << "Simulating";
  ns3::Simulator::Run();
  ns3::Simulator::Destroy();
  LOG(DEBUG) << "done.";
}
}
