#include <sma/ns3/ns3nodecontainer.hpp>
#include <sma/ns3/ns3inetlink.hpp>
#include <sma/link.hpp>

#include <sma/ccn/contentcache.hpp>

#include <sma/io/log>

#include <sma/async.hpp>
#include <sma/context.hpp>
#include <sma/chrono.hpp>

//#include <sma/utility.hpp>
#include <utility>

#include <ns3/ptr.h>
#include <ns3/uinteger.h>

#include <string>
#include <algorithm>


namespace sma
{
NS_OBJECT_ENSURE_REGISTERED(Ns3NodeContainer);

ns3::TypeId Ns3NodeContainer::TypeId()
{
  static ns3::TypeId tid
      = ns3::TypeId("sma::Ns3NodeContainer")
            .SetParent<ns3::Application>()
            .AddConstructor<Ns3NodeContainer>()
            .AddAttribute("id",
                          "The logical node's unique ID",
                          ns3::UintegerValue(0),
                          ns3::MakeUintegerAccessor(&Ns3NodeContainer::prop_id),
                          ns3::MakeUintegerChecker<std::uint32_t>());
  return tid;
}

Ns3NodeContainer::Ns3NodeContainer() {}

Ns3NodeContainer::~Ns3NodeContainer() {}
// Inherited from ns3::Application
void Ns3NodeContainer::DoDispose() {}

void Ns3NodeContainer::add_component(std::unique_ptr<Component> c)
{
  components.push_back(std::move(c));
}

void Ns3NodeContainer::act_schedule(Action& act)
{
  act.event_id = ns3::Simulator::Schedule(
      ns3::NanoSeconds(act.remaining_delay_ns().count()),
      &Ns3NodeContainer::act_next,
      this);
}

void Ns3NodeContainer::act_next()
{
  auto act = std::move(actions.front());
  actions.pop_front();
  (*act)();

  if (!actions.empty())
    act_schedule(*actions.front());
}

void Ns3NodeContainer::StartApplication()
{
  // Force the clock to be instantiated, setting the base time for the
  // simulation to the current real wall time. Since the simulation time is
  // given as an offset, all simulation times will be reported from this point.
  // At the time of writing this is actually done (wrongly...) by the
  // logger, but if e.g. logging was disabled then this would do the job.
  sma::chrono::system_clock::now();

  // Create network links and a manager to delegate messages to them.
  std::vector<std::unique_ptr<Link>> links;
  auto inet = static_cast<Link*>(new Ns3InetLink(GetNode()));
  links.emplace_back(inet);
  linklayer = std::make_unique<LinkLayerImpl>(std::move(links));

  NodeId node_id{prop_id};
  // Create the actor context to give the node node access to its environment.
  ctx = std::make_unique<Context>(std::string(node_id),
                                  static_cast<LinkLayer&>(*linklayer));

  fwd_strat = std::make_unique<PrForwardStrategy>(*ctx);

  // Add available device components to the context so the node can use them.
  for (auto& component : components)
    ctx->add_component(*component);

  node = std::make_unique<CcnNode>(node_id, *ctx);

  neighbor_helper = std::make_unique<NeighborHelperImpl>(*node);
  interest_helper = std::make_unique<InterestHelperImpl>(*node);
  behavior_helper = std::make_unique<BehaviorHelperImpl>(*node, 
           std::chrono::milliseconds(60000), 
           std::chrono::milliseconds(30000),
           std::chrono::milliseconds(60000));
  content_helper = std::make_unique<ContentHelperImpl>(*node);
  scheduler_helper = std::make_unique<ForwardSchedulerImpl>(*node, 1000);

  // Send received messages to the node
  linklayer->receive_to(*node);

  if (!actions.empty())
    act_schedule(*actions.front());
}

void Ns3NodeContainer::StopApplication()
{
  node->stop();
  Async::purge();
  linklayer->stop();
  node = nullptr;
  linklayer = nullptr;
}
}
