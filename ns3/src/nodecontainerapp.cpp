#include <sma/ns3/nodecontainerapp.hpp>

#include <sma/io/log>

#include <sma/link.hpp>
#include <sma/ns3/ns3inetlink.hpp>

#include <sma/async.hpp>
#include <sma/context.hpp>
#include <sma/chrono>

#include <ns3/ptr.h>
#include <ns3/uinteger.h>

#include <string>


namespace sma
{
NS_OBJECT_ENSURE_REGISTERED(Ns3NodeContainerApp);

ns3::TypeId Ns3NodeContainerApp::TypeId()
{
  static ns3::TypeId tid
      = ns3::TypeId("sma::Ns3NodeContainerApp")
            .SetParent<ns3::Application>()
            .AddConstructor<Ns3NodeContainerApp>()
            .AddAttribute(
                "id",
                "The logical node's unique ID",
                ns3::UintegerValue(0),
                ns3::MakeUintegerAccessor(&Ns3NodeContainerApp::prop_id),
                ns3::MakeUintegerChecker<std::uint32_t>());
  return tid;
}

/******************************************************************************
 * c/dtor and assignment
 */
Ns3NodeContainerApp::Ns3NodeContainerApp() {}
Ns3NodeContainerApp::Ns3NodeContainerApp(Ns3NodeContainerApp&& rhs) {}
Ns3NodeContainerApp& Ns3NodeContainerApp::operator=(Ns3NodeContainerApp&& rhs)
{
  return *this;
}
Ns3NodeContainerApp::~Ns3NodeContainerApp() { LOG(TRACE); }
// Inherited from ns3::Application; part of their lifecycle management I guess
void Ns3NodeContainerApp::DoDispose() {}
/* c/dtor and assignment
 *****************************************************************************/

void Ns3NodeContainerApp::add_component(std::unique_ptr<Component> c)
{
  components.push_back(std::move(c));
}

void Ns3NodeContainerApp::StartApplication()
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
  linklayer = std::make_unique<LinkLayer>(std::move(links));

  NodeId node_id{prop_id};
  // Create the actor context to give the node node access to its environment.
  ctx = std::make_unique<Context>(std::string(node_id));

  // Add available device components to the context so the node can use them.
  for (auto& component : components)
    ctx->add_component(component.get());

  node = std::make_unique<Node>(node_id, ctx.get());
  // Send received messages to the node
  linklayer->receive_to(node.get());
}

void Ns3NodeContainerApp::StopApplication()
{
  LOG(TRACE);
  linklayer->receive_to(nullptr);
  node = nullptr;
  linkmgr = nullptr;
}
}
