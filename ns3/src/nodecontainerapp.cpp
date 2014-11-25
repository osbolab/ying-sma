#include <sma/ns3/nodecontainerapp.hpp>

#include <sma/io/log>

// Link layer
#include <sma/link/linkmanager.hpp>
#include <sma/link/link.hpp>
#include <sma/link/ns3inetlink.hpp>

// Messaging layer
#include <sma/messagedispatch.hpp>

// Actor layer
#include <sma/async.hpp>
#include <sma/chrono>
#include <sma/context.hpp>
#include <sma/component.hpp>

// CCN application
#include <sma/ccn/node.hpp>

// NS3
#include <ns3/ptr.h>
#include <ns3/uinteger.h>
#include <ns3/application.h>

#include <cstdint>
#include <memory>
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
            .AddAttribute("id",
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
  linkmgr = std::make_unique<LinkManager>(std::move(links));

  // Create the messaging layer and connect it to the link manager
  msgr = std::make_unique<MessageDispatch>();
  msgr->outbox(linkmgr.get());
  linkmgr->inbox(msgr.get());

  // Create the actor context to give the node node access to its environment.
  NodeInfo ninfo{NodeId{prop_id}};
  // clang-format off
  ctx = std::make_unique<Context>(
      std::move(ninfo),
      msgr.get(),
      static_cast<Async*>(&async)
  );
  // clang-format on

  // Add available device components to the context so the node can use them.
  for (auto& component : components)
    ctx->add_component(component.get());

  // Create the node actor in the new context.
  node = std::make_unique<CcnNode>(ctx.get());
}

void Ns3NodeContainerApp::StopApplication()
{
  LOG(TRACE);
  if (node)
    node->dispose();
}
}
