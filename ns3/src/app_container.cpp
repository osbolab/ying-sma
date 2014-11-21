#include <sma/ns3/app_container.hpp>

#include <sma/app/application.hpp>

#include <sma/device.hpp>
#include <sma/device/inet_component.hpp>

#include <sma/network.hpp>
#include <sma/channel.hpp>
#include <sma/message_dispatch.hpp>

#include <sma/chrono>
#include <sma/unique_cast>
#include <sma/log>

#include <ns3/ptr.h>
#include <ns3/pointer.h>
#include <ns3/application.h>

#include <cstdint>
#include <memory>
#include <string>


namespace sma
{
ns3::TypeId app_container::TypeId()
{
  static ns3::TypeId tid = ns3::TypeId("sma::app_container")
                               .SetParent<ns3::Application>()
                               .AddConstructor<app_container>();
  return tid;
}

/******************************************************************************
 * c/dtor and assignment
 */
app_container::app_container()
  : dev(new device())
{
}
app_container::app_container(app_container&& rhs) {}
app_container& app_container::operator=(app_container&& rhs) { return *this; }
app_container::~app_container() {}
// Inherited from ns3::Application; part of their lifecycle management I guess
void app_container::DoDispose() { LOG(DEBUG); }
/* c/dtor and assignment
 *****************************************************************************/

void app_container::add_component(std::unique_ptr<component> c)
{
  dev->add_component(std::move(c));
}

void app_container::StartApplication()
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

void app_container::StopApplication()
{
  app->dispose();
  // Close the channel and, transitively, the sockets
  chan->close();
}
}
