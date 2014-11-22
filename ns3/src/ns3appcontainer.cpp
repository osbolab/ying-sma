#include <sma/ns3/ns3appcontainer.hpp>

#include <sma/link/linkmanager.hpp>
#include <sma/link/link.hpp>
#include <sma/link/ns3inetlink.hpp>

#include <sma/messagedispatch.hpp>
#include <sma/component.hpp>

#include <sma/ccn/application.hpp>

#include <sma/chrono>
#include <sma/unique_cast>
#include <sma/io/log>

#include <ns3/ptr.h>
#include <ns3/pointer.h>
#include <ns3/application.h>

#include <cstdint>
#include <memory>
#include <string>


namespace sma
{
ns3::TypeId Ns3AppContainer::TypeId()
{
  static ns3::TypeId tid = ns3::TypeId("sma::Ns3AppContainer")
                               .SetParent<ns3::Application>()
                               .AddConstructor<Ns3AppContainer>();
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
Ns3AppContainer::~Ns3AppContainer() {}
// Inherited from ns3::Application; part of their lifecycle management I guess
void Ns3AppContainer::DoDispose() { LOG(DEBUG); }
/* c/dtor and assignment
 *****************************************************************************/

void Ns3AppContainer::add_component(std::unique_ptr<Component> c) {}

void Ns3AppContainer::StartApplication()
{
  // Force the clock to use the current real wall time as the beginning
  // of the simulation.
  sma::chrono::system_clock::now();

  std::vector<std::unique_ptr<Link>> links;
  auto inet = static_cast<Link*>(new Ns3InetLink(GetNode()));
  links.emplace_back(inet);
  linkmgr = std::make_unique<LinkManager>(std::move(links));

  msgr = std::make_unique<MessageDispatch>();
  msgr->outbox(linkmgr.get());
  linkmgr->inbox(msgr.get());
}

void Ns3AppContainer::StopApplication() {}
}
