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
	
#include <sma/nodeid.hpp>

#include <ns3/core-module.h>
#include <ns3/wifi-module.h>
#include <ns3/olsr-module.h>
#include <ns3/config-store-module.h>
#include <ns3/internet-module.h>
#include <ns3/application-container.h>
#include <ns3/mobility-helper.h>
#include <ns3/mobility-model.h>
#include <ns3/uinteger.h>
#include <ns3/netanim-module.h>
#include <ns3/flow-monitor-module.h>
//#include <ns3/constant-position-mobility-model.h>
#include <ns3/stats-module.h>

#include <random>
#include <string>
#include <cstring>
#include <iostream>
#include <chrono>
#include <sstream>
#include <ctime>

    using namespace std::literals::chrono_literals;


void configure_logs(int& argc, char** argv);

void add_stats_on_node (ns3::DataCollector& data, uint16_t node_id);

void TxCallback (ns3::Ptr<ns3::CounterCalculator<uint32_t>>datac,
                 std::string path, ns3::Ptr<const ns3::Packet> packet);
				 
void RxCallback (ns3::Ptr<ns3::CounterCalculator<uint32_t>>datac,
                 std::string path, ns3::Ptr<const ns3::Packet> packet);

int main(int argc, char** argv)
{
  configure_logs(argc, argv);

  std::size_t nnodes = 50;
  long duration = 200;

  std::string baseIp("10.1.0.0");
  std::string subnet("255.255.0.0");

  bool enable_olsr = false;
  std::string phyMode("DsssRate1Mbps");
  double rss = -80.0;        // -dBm
  double distance = 100;    // m
  std::string fragmentThreshold = "2200";
  std::uint16_t packet_size = 64;

  bool arq = true;
  bool enableFlowMonitor = true;
  std::string animFile = "random-walk.xml";  // Name of file for animation output
  
  std::string format ("omnet");
  std::string experiment ("random-walk");
  std::string strategy ("friis model");
  std::string input;
  std::string runID;

  ns3::CommandLine cmd;
  cmd.AddValue("phyMode", "Wifi physical mode", phyMode);
  cmd.AddValue("rss", "Received Signal Strength", rss);
  cmd.AddValue("distance", "distance (m)", distance);
  cmd.AddValue("nodes", "number of nodes", nnodes);
  cmd.AddValue("packet", "packet size", packet_size);
  cmd.AddValue("olsr", "enable optimized link state routing", enable_olsr);
  cmd.AddValue("animFile", "File Name for Animation Output", animFile);
  cmd.AddValue ("arq", "whether to use arq", arq);
  cmd.AddValue("EnableFlowMonitor", "Enable Flow Monitor", enableFlowMonitor);
  // Add parameters related to statistics framework.
  cmd.AddValue("format", "Format to use for data output.", format);
  cmd.AddValue("experiment", "Identifier for experiment.", experiment);
  cmd.AddValue("strategy", "Identifier for strategy.", strategy);
  cmd.AddValue("run", "Identifier for run.", runID);
  cmd.Parse(argc, argv);


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

  wifiPhy.Set("TxPowerStart", ns3::DoubleValue(10));
  wifiPhy.Set("TxPowerEnd", ns3::DoubleValue(10));

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
                                ns3::DoubleValue(0.01),
                                "MinY",
                                ns3::DoubleValue(0.01),
                                "DeltaX",
                                ns3::DoubleValue(distance),
                                "DeltaY",
                                ns3::DoubleValue(distance),
                                "GridWidth",
                                ns3::UintegerValue(3),
                                "LayoutType",
                                ns3::StringValue("RowFirst"));

  
//  LOG(DEBUG) << "  - Position: uniform random constrained";
//  std::uniform_real_distribution<double> lat_dist(40.68, 40.69);
//  std::uniform_real_distribution<double> lon_dist(-74, -73.99);
//  std::default_random_engine rnd;

//  auto posAlloc = ns3::CreateObject<ns3::ListPositionAllocator>();
//  for (std::size_t i = 0; i < nnodes; ++i) {
//    auto lat = lat_dist(rnd);
//    auto lon = lon_dist(rnd);
//    LOG(DEBUG) << "    > lat: " << lat << ", lon: " << lon;
//    posAlloc->Add(ns3::Vector(lat, 0.0, lon));
//  }
//  mobility.SetPositionAllocator(posAlloc);
  

//  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
//                             "Mode", ns3::StringValue ("Time"),
//                             "Time", ns3::StringValue ("1s"),
//                             "Speed", ns3::StringValue ("ns3::ConstantRandomVariable[Constant=20.0]"),
//                             "Bounds", ns3::StringValue ("0|1000|0|1000"));
							 
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

  // Generate random events
  std::unordered_map<size_t, std::vector<int>> event_table;
  const int INTERVAL = 5;
  int num_of_content_generated = duration/INTERVAL - 4;
  srand(time(NULL));
  while (num_of_content_generated-->0) {
    int random_node_id = rand() % nnodes;
    auto result = event_table.emplace(static_cast<size_t>(random_node_id), 
            std::vector<int>(1, duration/INTERVAL-3 - num_of_content_generated));
    if (!result.second)
      result.first->second.push_back((duration/INTERVAL-3 - num_of_content_generated)*INTERVAL);
  }


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

    auto result = event_table.find(i);
    if (result != event_table.end()) {
        std::vector<int> events = result->second;
        for (std::vector<int>::const_iterator it = events.begin();
                it != events.end(); it++) {
          app->act_emplace_front<sma::PublishContentAction>(
                  std::chrono::seconds(*it), "cats", std::to_string(*it));
        }
    }

//    if (i == (nnodes - 1)) {
      std::vector<sma::ContentType> interests;
      interests.emplace_back("cats");
      app->act_emplace_front<sma::CreateInterestAction>(1s,
                                                        std::move(interests));
//    }
//    if (i == 0) {
//      std::vector<sma::ContentType> interests;
//      interests.emplace_back("dogs");
     // // app->act_emplace_front<sma::CreateInterestAction>(3s,
     // //                                                  std::move(interests));
//      char namebuf[packet_size];
//      std::memset(namebuf, 'a', packet_size);
//      app->act_emplace_front<sma::PublishContentAction>(
//          3s, "cats", std::string(namebuf, packet_size));
//    }

    auto apps = ns3::ApplicationContainer(app);
    apps.Start(ns3::Seconds(0));
    apps.Stop(ns3::Seconds(duration));

    node->AddApplication(app);
  }
  LOG(INFO) << "Created " << nnodes << " nodes";

  // ^^^^^^^^^^^^^^^^^^^^^^^^^ SMA STUFF ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

  ns3::AsciiTraceHelper ascii;
  wifiPhy.EnableAsciiAll (ascii.CreateFileStream ("log/random-walk.tr"));
  wifiPhy.EnablePcap("log/random-walk", devices);

  if (enable_olsr) {
    auto routeStream = ns3::Create<ns3::OutputStreamWrapper>(
        "log/random-walk.routes", std::ios::out);
    olsr.PrintRoutingTableAllEvery(ns3::Seconds(10), routeStream);
  }

//  LOG(DEBUG)
//      << "Listing neighbors every 2 seconds to log/wifi-simple.neighbors";
//  auto neighborStream = ns3::Create<ns3::OutputStreamWrapper>(
//      "log/wifi-simple.neighbors", std::ios::out);
//  olsr.PrintNeighborCacheAllEvery(ns3::Seconds(2), neighborStream);

  // Flow monitor
//  ns3::FlowMonitorHelper flowmon;
//  ns3::Ptr<ns3::FlowMonitor> monitor = flowmon.InstallAll();
//  monitor->SetAttribute("DelayBinWidth", ns3::DoubleValue(0.001));
//  monitor->SetAttribute("JitterBinWidth", ns3::DoubleValue(0.001));
//  monitor->SetAttribute("PacketSizeBinWidth", ns3::DoubleValue(1000));

  LOG(WARNING) << "Simulating " << duration << " seconds";
  ns3::Simulator::Stop(ns3::Seconds(duration + 5));

  // Create the animation object and configure for specified output
  ns3::AnimationInterface anim (animFile);
  anim.SetMobilityPollInterval (ns3::Seconds (2));
//  anim.EnablePacketMetadata (); // Optional
//  anim.EnableIpv4L3ProtocolCounters (ns3::Seconds (0), ns3::Seconds (10)); // Optional
  
  //---------------------------------------------------------------------------
  //-- Setup stats and data collection
  //---------------------------------------------------------------------------



  std::stringstream sstr ("");
  sstr << nnodes
	   << "-"
	   << "distance";
  input = sstr.str();

  ns3::DataCollector data;
  data.DescribeRun (experiment,
                    strategy,
					input,
					runID);
  
  
  add_stats_on_node(data, 0);
  add_stats_on_node(data, 1);
  add_stats_on_node(data, 2);
  add_stats_on_node(data, 3);
  add_stats_on_node(data, 4);
  add_stats_on_node(data, 5);
  add_stats_on_node(data, 6);
  add_stats_on_node(data, 7);
  add_stats_on_node(data, 8);
  //---------------------------------------------------------------------------
  //-- Run the simulation
  //---------------------------------------------------------------------------

  ns3::Simulator::Run();

/*
  // Print per flow statistics
  monitor->CheckForLostPackets ();
  ns3::Ptr<ns3::Ipv4FlowClassifier> classifier = ns3::DynamicCast<ns3::Ipv4FlowClassifier> (flowmon.GetClassifier ());
  std::map<ns3::FlowId, ns3::FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
  LOG(DEBUG) << stats.size();
  uint32_t totalRx =0;
  uint32_t totalTx =0;
  uint32_t totalDrop =0;
  double delay = 0;
  double count =0;
  double hopCount = 0;
  for (std::map<ns3::FlowId, ns3::FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
  {
      ns3::Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
      if (i->second.rxPackets != 0){
         totalRx +=  i->second.rxPackets;
         totalTx +=  i->second.txPackets;
         hopCount += (i->second.timesForwarded / i->second.rxPackets +1);
         delay += (i->second.delaySum.GetSeconds() / i->second.rxPackets);
         count++;
         for (uint32_t j=0; j < i->second.packetsDropped.size() ; j++){
           totalDrop += i->second.packetsDropped[j];
         }
       }
  }
  LOG(INFO) << "  total Rx = " << (double)totalRx;
  LOG(INFO) << "  total Tx = " << (double)totalTx;
  LOG(INFO) << "  PDR = " << ((double)totalRx / (double)totalTx)*100.0;
  LOG(INFO) << "  hopCount = " << hopCount;
  LOG(INFO) << "  count = " << count;
  LOG(INFO) << "  Mean HOP count = " << hopCount / count;
  LOG(INFO) << "  Mean Delay = " << delay / count;
  LOG(INFO) << "  L3 Drop Packets = " << totalDrop;

  monitor->SerializeToXmlFile("FlowMonitor.xml", true, true);


*/
  
  

  LOG(WARNING) << "Simulation ended";
  LOG(INFO) << "Animation Trace file created:" << animFile.c_str();
 
  
  //---------------------------------------------------------------------------
  //-- Generate statistics output.
  //---------------------------------------------------------------------------
  
  
  ns3::Ptr<ns3::DataOutputInterface> output = 0;
  if (format == "omnet") {
	LOG(INFO) << "Creating omnet formatted data output.";
	output = ns3::CreateObject<ns3::OmnetDataOutput>();
  } else if (format == "db") {
#ifdef STATS_HAS_SQLITE3
	  LOG(INFO) << "Creating sqlite formatted dataoutput.";
	  output = ns3::CreateObject<ns3::SqliteDataOutput>();
#endif
  } else {
	  LOG(INFO) << "Unknown output format " << format;
  }
  
  if (output != 0)
	output->Output (data);
  
  //---------------------------------------------------------------------------
  //-- End of simulation
  //---------------------------------------------------------------------------
  
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

void add_stats_on_node (ns3::DataCollector& data, uint16_t node_id)
{

					
    ns3::Ptr<ns3::CounterCalculator<uint32_t>> totalTx = 
  	  ns3::CreateObject<ns3::CounterCalculator<uint32_t>>();
  
  
    totalTx->SetKey ("random-walk-tx-frame");
    std::stringstream sstr ("");
    sstr << node_id;
    totalTx->SetContext (sstr.str());
    sstr.str("");
  
    sstr << "/NodeList/" << node_id << "/DeviceList/*/$ns3::WifiNetDevice/Mac/MacTx";
    LOG(INFO) << "NODELIST: " << sstr.str();
  
    ns3::Config::Connect (sstr.str(),
                        ns3::MakeBoundCallback (&TxCallback, totalTx));
  //  ns3::Config::Connect ("/NodeList/0/DeviceList/*/$ns3::WifiNetDevice/Mac/MacTx",
  //				   ns3::MakeBoundCallback (&TxCallback, totalTx));
    data.AddDataCalculator (totalTx);
  
    ns3::Ptr<ns3::CounterCalculator<uint32_t>> totalRx = 
  	  ns3::CreateObject<ns3::CounterCalculator<uint32_t>>();
    totalRx->SetKey ("random-walk-rx-frame");
    sstr.str("");
    sstr << node_id;
    totalRx->SetContext (sstr.str());
    sstr.str("");
  
    sstr << "/NodeList/" << node_id << "/DeviceList/*/$ns3::WifiNetDevice/Mac/MacRx",
    ns3::Config::Connect (sstr.str(),
    					ns3::MakeBoundCallback(&RxCallback, totalRx));
    data.AddDataCalculator (totalRx);
    // Calculate the number of frames received by at least one node
  //  auto node = nodes.Get(id_to_check);
  //  auto app_installed = ns3::DynamicCast<sma::Ns3NodeContainer> (node->GetApplication(0)); // only one app is installed
  //  std::vector<sma::NodeId> neighbor_ids = app_installed->get_neighbor_ids();
  //  LOG(INFO) << "The # of neighbors of node " << id_to_check << " is " << neighbor_ids.size();
}

void TxCallback (ns3::Ptr<ns3::CounterCalculator<uint32_t>>datac,
                 std::string path, ns3::Ptr<const ns3::Packet> packet) 
{
  LOG(INFO) << "Frame transmitted";
  LOG(INFO) << "Sent frame counted in " << datac->GetKey();
  datac->Update();           	
}

void RxCallback (ns3::Ptr<ns3::CounterCalculator<uint32_t>>datac,
                 std::string path, ns3::Ptr<const ns3::Packet> packet)
{
  LOG(INFO) << "Frame recieved";
  LOG(INFO) << "Received frame counted in " << datac->GetKey();
  datac->Update();				 	
}
