#include <sma/ns3/ns3appcontainer.hpp>

#include <sma/link/linkmanager.hpp>
#include <sma/link/link.hpp>
#include <sma/link/ns3inetlink.hpp>

#include <sma/messagedispatch.hpp>
#include <sma/component.hpp>

#include <sma/async.hpp>
#include <sma/context.hpp>
#include <sma/ccn/ccnapplication.hpp>

#include <sma/chrono>
#include <sma/unique_cast>
#include <sma/io/log>

#include <ns3/ptr.h>
#include <ns3/uinteger.h>
#include <ns3/application.h>

#include <cstdint>
#include <memory>
#include <string>


namespace sma
{
NS_OBJECT_ENSURE_REGISTERED(Ns3AppContainer);

ns3::TypeId Ns3AppContainer::TypeId()
{
  static ns3::TypeId tid
      = ns3::TypeId("sma::Ns3AppContainer")
            .SetParent<ns3::Application>()
            .AddConstructor<Ns3AppContainer>()
            .AddAttribute("id",
                          "The logical node's unique ID",
                          ns3::UintegerValue(0),
                          ns3::MakeUintegerAccessor(&Ns3AppContainer::prop_id),
                          ns3::MakeUintegerChecker<std::uint32_t>());
  return tid;
}

/******************************************************************************
 * c/dtor and assignment
 */
Ns3AppContainer::Ns3AppContainer() {}
Ns3AppContainer::Ns3AppContainer(Ns3AppContainer&& rhs) {}
Ns3AppContainer& Ns3AppContainer::operator=(Ns3AppContainer&& rhs)
{
  return *this;
}
Ns3AppContainer::~Ns3AppContainer() { LOG(TRACE); }
// Inherited from ns3::Application; part of their lifecycle management I guess
void Ns3AppContainer::DoDispose()
{
  LOG(WARNING) << "Application container dying";
}
/* c/dtor and assignment
 *****************************************************************************/

void Ns3AppContainer::add_component(std::unique_ptr<Component> c) {}

void Ns3AppContainer::StartApplication()
{
  // Force the clock to be instantiated, setting the base time for the
  // simulation to the current real wall time. Since the simulation time is
  // given as an offset, all simulation times will be reported from this point.
  // At the time of writing this is actually done (wrongly...) by the
  // logger, but if e.g. logging was disabled then this would do the job.
  sma::chrono::system_clock::now();

  std::vector<std::unique_ptr<Link>> links;
  auto inet = static_cast<Link*>(new Ns3InetLink(GetNode()));
  links.emplace_back(inet);
  LOG(DEBUG) << "created " << links.size() << " network link(s)";
  linkmgr = std::make_unique<LinkManager>(std::move(links));

  msgr = std::make_unique<MessageDispatch>();
  msgr->outbox(linkmgr.get());
  linkmgr->inbox(msgr.get());

  NodeInfo ninfo{NodeId{prop_id}};
  ctx = std::make_unique<Context>(
      std::move(ninfo), msgr.get(), static_cast<Async*>(&async));

  app = std::make_unique<CcnApplication>(ctx.get());
}

void Ns3AppContainer::StopApplication()
{
  LOG(DEBUG) << "Stopping";
  if (app)
    app->dispose();
}
}
