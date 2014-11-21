#include <sma/ns3/ns3appcontainer.hpp>

#include <sma/ccn/application.hpp>

#include <sma/device.hpp>
#include <sma/device/inet_component.hpp>

#include <sma/network.hpp>
#include <sma/channel.hpp>
#include <sma/message_dispatch.hpp>

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
Ns3AppContainer::Ns3AppContainer()
  : dev(new device())
{
}
Ns3AppContainer::Ns3AppContainer(Ns3AppContainer&& rhs) {}
Ns3AppContainer& Ns3AppContainer::operator=(Ns3AppContainer&& rhs) { return *this; }
Ns3AppContainer::~Ns3AppContainer() {}
// Inherited from ns3::Application; part of their lifecycle management I guess
void Ns3AppContainer::DoDispose() { LOG(DEBUG); }
/* c/dtor and assignment
 *****************************************************************************/

void Ns3AppContainer::add_component(std::unique_ptr<component> c)
{
  dev->add_component(std::move(c));
}

void Ns3AppContainer::StartApplication()
{
  // Force the clock to use the current real wall time as the beginning
  // of the simulation.
  sma::chrono::system_clock::now();

  assert(dev);

  auto inet = dev->try_get<ns3_inet_component>();
  assert(inet);

  msgr = message_dispatch::new_single_threaded();

  net = std::make_unique<network>(dev.get());
  net->heavy_chan->inbox(msgr.get());
  msgr->add_outbox(net->heavy_chan);

  context ctx(dev.get(), msgr.get());
  app = std::make_unique<application>(std::move(ctx));
}

void Ns3AppContainer::StopApplication()
{
  app->dispose();
  // Close the channel and, transitively, the sockets
  chan->close();
}
}
