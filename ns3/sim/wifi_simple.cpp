#define _ELPP_NO_DEFAULT_LOG_FILE
#define _ELPP_LOGGING_FLAGS_FROM_ARG
#include <sma/io/log>
_INITIALIZE_EASYLOGGINGPP    // Call only once per application

#include <sma/gps.hpp>
#include <sma/dummygps.hpp>
#include <sma/ns3/ns3nodecontainer.hpp>
#include <sma/ns3/action.hpp>
#include <sma/ns3/createinterestaction.hpp>
#include <sma/ns3/publishcontentaction.hpp>

#include <ns3/core-module.h>
#include <ns3/wifi-module.h>
#include <ns3/olsr-module.h>
#include <ns3/config-store-module.h>
#include <ns3/internet-module.h>
#include <ns3/application-container.h>
#include <ns3/mobility-helper.h>
#include <ns3/mobility-model.h>
#include <ns3/uinteger.h>

#include <random>
#include <iostream>
#include <chrono>

    using namespace std::literals::chrono_literals;


void configure_logs(int& argc, char** argv);

int main(int argc, char** argv)
{
  configure_logs(argc, argv);

  std::size_t nnodes = 4;
  long duration = 30;

  std::string baseIp("10.1.0.0");
  std::string subnet("255.255.0.0");

  bool enable_olsr = false;
  std::string phyMode("DsssRate1Mbps");
  double rss = -80.0;        // -dBm
  double distance = 2000;    // m
  std::string fragmentThreshold = "2200";

  ns3::CommandLine cmd;
  cmd.AddValue("phyMode", "Wifi physical mode", phyMode);
  cmd.AddValue("rss", "Received Signal Strength", rss);
  cmd.AddValue("distance", "distance (m)", distance);
  cmd.AddValue("nodes", "number of nodes", nnodes);
  cmd.AddValue("olsr", "enable optimized link state routing", enable_olsr);
  // cmd.Parse(argc, argv);


  LOG(DEBUG) << "Create 802.11b device template";
  // clang-format off
  // Don't fragment frames < 2200 bytes
  ns3::Config::SetDefault("ns3::WifiRemoteStationManager::FragmentationThreshold",
                          ns3::StringValue(fragmentThreshold));
  LOG(DEBUG) << "  - Fragmentation lower limit: " << fragmentThreshold << " bytes";
  // Turn off RTS/CTS for frames < 2200 bytes
  ns3::Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold",
                          ns3::StringValue(fragmentThreshold));
  LOG(DEBUG) << "  - RTS/CTS disabled below: " << fragmentThreshold << " bytes";
  // Adjust non-unicast data rate to be the same as unicast
  ns3::Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode",
                          ns3::StringValue(phyMode));
  LOG(DEBUG) << "  - Multicast/broadcast unthrottled";

  // clang-format on
  ns3::WifiHelper wifi;
  // wifi.EnableLogComponents();
  wifi.SetStandard(ns3::WIFI_PHY_STANDARD_80211b);

  auto wifiPhy = ns3::YansWifiPhyHelper::Default();
  // When using a fixed RSS model this should be zero lest gain be added
  wifiPhy.Set("RxGain", ns3::DoubleValue(0));
  LOG(DEBUG) << "  - Rx gain: 0 dBm";
  // Something about RadioTap and Prism tracing
  wifiPhy.SetPcapDataLinkType(ns3::YansWifiPhyHelper::DLT_IEEE802_11_RADIO);

  ns3::YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  LOG(DEBUG) << "  - Propagation delay: constant";
  // Receive strength is fixed regardless of distance or TX power
  wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel");
  LOG(DEBUG) << "  - Propagation loss model: Friis";

  wifiPhy.SetChannel(wifiChannel.Create());
  LOG(DEBUG) << "802.11b physical model created";

  auto wifiMac = ns3::NqosWifiMacHelper::Default();
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                               "DataMode",
                               ns3::StringValue(phyMode),
                               "ControlMode",
                               ns3::StringValue(phyMode));
  wifiMac.SetType("ns3::AdhocWifiMac");

  LOG(DEBUG) << "WiFi MAC:";
  LOG(DEBUG) << "  - QoS: none";
  LOG(DEBUG) << "  - Data mode: " << phyMode;
  LOG(DEBUG) << "  - Control mode: " << phyMode;
  LOG(DEBUG) << "  - Rate: constant";
  LOG(DEBUG) << "  - Mode: adhoc";

  LOG(DEBUG) << "Create " << nnodes << " virtual WiFi devices";
  LOG(DEBUG) << "  - Mobility: fixed";
  LOG(DEBUG) << "  - Position: grid (" << distance << "m distance)";
  ns3::NodeContainer nodes;
  nodes.Create(nnodes);
  auto devices = wifi.Install(wifiPhy, wifiMac, nodes);

  ns3::MobilityHelper mobility;
  mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                "MinX",
                                ns3::DoubleValue(0.0),
                                "MinY",
                                ns3::DoubleValue(0.0),
                                "DeltaX",
                                ns3::DoubleValue(distance),
                                "DeltaY",
                                ns3::DoubleValue(distance),
                                "GridWidth",
                                ns3::UintegerValue(1),
                                "LayoutType",
                                ns3::StringValue("RowFirst"));
  /*
  LOG(DEBUG) << "  - Position: uniform random constrained";
  std::uniform_real_distribution<double> lat_dist(40.68, 40.69);
  std::uniform_real_distribution<double> lon_dist(-74, -73.99);
  std::default_random_engine rnd;

  auto posAlloc = ns3::CreateObject<ns3::ListPositionAllocator>();
  for (std::size_t i = 0; i < nnodes; ++i) {
    auto lat = lat_dist(rnd);
    auto lon = lon_dist(rnd);
    LOG(DEBUG) << "    > lat: " << lat << ", lon: " << lon;
    posAlloc->Add(ns3::Vector(lat, 0.0, lon));
  }
  mobility.SetPositionAllocator(posAlloc);
  */
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(nodes);

  LOG(DEBUG) << "Create Internet and assign IP addresses to devices";
  LOG(DEBUG) << "  - Base IP: " << baseIp;
  LOG(DEBUG) << "  - Subnet: " << subnet;
  ns3::OlsrHelper olsr;
  ns3::Ipv4ListRoutingHelper list;
  if (enable_olsr) {
    LOG(DEBUG) << "  - Routing: OLSR";
    ns3::Ipv4StaticRoutingHelper staticRouting;
    list.Add(staticRouting, 0);
    list.Add(olsr, 10);
  }

  ns3::InternetStackHelper internet;
  if (enable_olsr)
    internet.SetRoutingHelper(list);
  internet.Install(nodes);
  ns3::Ipv4AddressHelper ip;
  ip.SetBase(baseIp.c_str(), subnet.c_str());
  ns3::Ipv4InterfaceContainer inet_addrs = ip.Assign(devices);

  // ------------------------- SMA STUFF --------------------------------
  // NS3 gives us some dependency injection when creating applications.
  // The application in this case is an NS3 wrapper around some executable
  // component so NS3 can drive it; that component is our SMA application.
  // NS3 drives the application by supplying timed signals to the scheduler
  // and packets to the sockets; the sockets in turn call up through the
  // channels to the messengers which deliver messages to the high-level
  // application.
  LOG(DEBUG) << "Install SMA application instances in virtual nodes";
  ns3::ObjectFactory sma_factory;
  sma_factory.SetTypeId(sma::Ns3NodeContainer::TypeId());


  // We can keep using that injection template to spawn applications and
  // attach them to nodes.
  for (std::size_t i = 0; i < nnodes; ++i) {
    auto node = nodes.Get(i);
    auto mob = node->GetObject<ns3::MobilityModel>();
    auto pos = mob->GetPosition();

    auto app = sma_factory.Create<sma::Ns3NodeContainer>();
    app->SetAttribute("id", ns3::UintegerValue(i));

    app->add_component(std::move(
        std::make_unique<sma::DummyGps>(sma::GPS::Coord{pos.x, pos.y})));

    if (i == 1) {
      std::vector<sma::ContentType> interests;
      interests.emplace_back("cats");
      app->act_emplace_front<sma::CreateInterestAction>(1s,
                                                        std::move(interests));
      app->act_emplace_front<sma::PublishContentAction>(2s, "cats", "my cat");
    }
    if (i == 7) {
      std::vector<sma::ContentType> interests;
      interests.emplace_back("dogs");
      app->act_emplace_front<sma::CreateInterestAction>(3s,
                                                        std::move(interests));
    }

    auto apps = ns3::ApplicationContainer(app);
    apps.Start(ns3::Seconds(0));
    apps.Stop(ns3::Seconds(duration));

    node->AddApplication(app);
  }
  LOG(INFO) << "Created " << nnodes << " nodes";
  // ^^^^^^^^^^^^^^^^^^^^^^^^^ SMA STUFF ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

  LOG(DEBUG) << "Traffic will output to log/wifi_simple-x-x.pcap";
  LOG(DEBUG) << "Tracing routes every 10 seconds to log/wifi-simple.routes";
  ns3::AsciiTraceHelper ascii;
  wifiPhy.EnablePcap("log/wifi_simple", devices);

  if (enable_olsr) {
    auto routeStream = ns3::Create<ns3::OutputStreamWrapper>(
        "log/wifi-simple.routes", std::ios::out);
    olsr.PrintRoutingTableAllEvery(ns3::Seconds(10), routeStream);
  }

  /*
  LOG(DEBUG)
      << "Listing neighbors every 2 seconds to log/wifi-simple.neighbors";
  auto neighborStream = ns3::Create<ns3::OutputStreamWrapper>(
      "log/wifi-simple.neighbors", std::ios::out);
  olsr.PrintNeighborCacheAllEvery(ns3::Seconds(2), neighborStream);
  */

  LOG(WARNING) << "Simulating " << duration << " seconds";
  ns3::Simulator::Stop(ns3::Seconds(duration + 5));
  ns3::Simulator::Run();
  LOG(WARNING) << "Simulation ended";
  ns3::Simulator::Destroy();
  LOG(INFO) << "done.";
}

void configure_logs(int& argc, char** argv)
{
  _START_EASYLOGGINGPP(argc, argv);

  std::cout << "Configuring application logging from log.conf...\n";
  el::Configurations logconf("../../conf/log.conf");
  el::Loggers::reconfigureAllLoggers(logconf);
  LOG(INFO) << "\n\n----------------------------------- session "
               "-----------------------------------";
  LOG(DEBUG) << "Configuring node logging from nodelog.conf...";
  el::Configurations nodelogconf("../../conf/nodelog.conf");
  el::Loggers::setDefaultConfigurations(nodelogconf, false);
  el::Loggers::getLogger("nodes");
  CLOG(INFO, "nodes") << "------------------------- session "
                         "-------------------------";

  el::Loggers::addFlag(el::LoggingFlag::LogDetailedCrashReason);
  LOG(DEBUG) << "Detailed crash reports are enabled (--logging-flags=4)";
  el::Loggers::addFlag(el::LoggingFlag::ColoredTerminalOutput);
  LOG(DEBUG) << "Colored terminal output is enabled (--logging-flags=64)";
}
