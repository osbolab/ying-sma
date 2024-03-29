#define _ELPP_NO_DEFAULT_LOG_FILE
#define _ELPP_LOGGING_FLAGS_FROM_ARG
#include <sma/io/log>
_INITIALIZE_EASYLOGGINGPP    // Call only once per application

#include <sma/stats.hpp>

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
#include <string>
#include <cstring>
#include <iostream>
#include <chrono>

    using namespace std::literals::chrono_literals;


void configure_logs(int& argc, char** argv);

int main(int argc, char** argv)
{
  configure_logs(argc, argv);

  std::size_t nnodes = 2;
  long duration = 60;

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
  cmd.Parse(argc, argv);


  // clang-format off
  // Don't fragment frames < 2200 bytes
  ns3::Config::SetDefault("ns3::WifiRemoteStationManager::FragmentationThreshold",
                          ns3::StringValue(fragmentThreshold));
  // Turn off RTS/CTS for frames < 2200 bytes
  ns3::Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold",
                          ns3::StringValue(fragmentThreshold));
  // Adjust non-unicast data rate to be the same as unicast
  ns3::Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode",
                          ns3::StringValue(phyMode));

  // clang-format on
  ns3::WifiHelper wifi;
  // wifi.EnableLogComponents();
  wifi.SetStandard(ns3::WIFI_PHY_STANDARD_80211b);

  auto wifiPhy = ns3::YansWifiPhyHelper::Default();
  // When using a fixed RSS model this should be zero lest gain be added
  wifiPhy.Set("RxGain", ns3::DoubleValue(0));
  // Something about RadioTap and Prism tracing
  wifiPhy.SetPcapDataLinkType(ns3::YansWifiPhyHelper::DLT_IEEE802_11_RADIO);

  ns3::YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  // Receive strength is fixed regardless of distance or TX power
  wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel");

  wifiPhy.SetChannel(wifiChannel.Create());

  auto wifiMac = ns3::NqosWifiMacHelper::Default();
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                               "DataMode",
                               ns3::StringValue(phyMode),
                               "ControlMode",
                               ns3::StringValue(phyMode));
  wifiMac.SetType("ns3::AdhocWifiMac");

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

  ns3::OlsrHelper olsr;
  ns3::Ipv4ListRoutingHelper list;
  if (enable_olsr) {
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
        std::make_unique<sma::DummyGps>(mob)));

    if (i == (nnodes - 1)) {
      std::vector<sma::ContentType> interests;
      interests.emplace_back("cats");
      app->act_emplace_front<sma::CreateInterestAction>(1s,
                                                        std::move(interests));
    }
    if (i == 0) {
      std::vector<sma::ContentType> interests;
      interests.emplace_back("dogs");
      // app->act_emplace_front<sma::CreateInterestAction>(3s,
      //                                                  std::move(interests));
      app->act_emplace_front<sma::PublishContentAction>(
          3s, "cats", "my cool cat");
    }

    auto apps = ns3::ApplicationContainer(app);
    apps.Start(ns3::Seconds(0));
    apps.Stop(ns3::Seconds(duration));

    node->AddApplication(app);
  }
  // ^^^^^^^^^^^^^^^^^^^^^^^^^ SMA STUFF ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

  ns3::AsciiTraceHelper ascii;
  wifiPhy.EnablePcap("/var/log/sma/pcap/wifi_simple", devices);

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
  sma::stats::print_stats();

  ns3::Simulator::Run();
  LOG(WARNING) << "Simulation ended";
  ns3::Simulator::Destroy();
  LOG(INFO) << "done.";
}

void configure_logs(int& argc, char** argv)
{
  _START_EASYLOGGINGPP(argc, argv);

  std::cout << "Configuring application logging from log.conf...\n";
  el::Configurations logconf("../conf/log.conf");
  el::Loggers::reconfigureAllLoggers(logconf);
  LOG(INFO) << "\n\n----------------------------------- session "
               "-----------------------------------";
  LOG(DEBUG) << "Configuring node logging from nodelog.conf...";
  el::Configurations nodelogconf("../conf/nodelog.conf");
  el::Loggers::setDefaultConfigurations(nodelogconf, false);
  el::Loggers::getLogger("nodes");
  CLOG(INFO, "nodes") << "------------------------- session "
                         "-------------------------";

  el::Loggers::addFlag(el::LoggingFlag::LogDetailedCrashReason);
  LOG(DEBUG) << "Detailed crash reports are enabled (--logging-flags=4)";
  el::Loggers::addFlag(el::LoggingFlag::ColoredTerminalOutput);
  LOG(DEBUG) << "Colored terminal output is enabled (--logging-flags=64)";
}
