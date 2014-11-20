#include <sma/ns3/app_container.hpp>

#include <sma/app/application.hpp>

#include <sma/device.hpp>
#include <sma/channel.hpp>
#include <sma/message_dispatch.hpp>
#include <sma/chrono>
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
  static ns3::TypeId tid
      = ns3::TypeId("sma::app_container")
            .SetParent<ns3::Application>()
            .AddConstructor<app_container>();
  return tid;
}

/******************************************************************************
 * c/dtor and assignment
 */
app_container::app_container() {}
app_container::app_container(Myt&& rhs) {}
app_container& app_container::operator=(Myt&& rhs) { return *this; }
app_container::~app_container() {}
// Inherited from ns3::Application; part of their lifecycle management I guess
void app_container::DoDispose() { LOG(DEBUG); }
/* c/dtor and assignment
 *****************************************************************************/

void app_container::StartApplication()
{
  assert(dev);
  // Force the clock to use the current real wall time as the beginning
  // of the simulation.
  sma::chrono::system_clock::now();

  // Create an endpoint for the messaging to send and receive through
  sock = std::make_unique<ns3_socket>(GetNode());
  sock->bind(socket_addr("0.0.0.0", 9999));
  chan = std::make_unique<ns3_channel>(sock.get());

  msgr = message_dispatch::new_single_threaded(chan.get());
  chan->inbox(msgr.get());

  context ctx{dev.get(), msgr.get() };
  app = std::make_unique<application>(std::move(ctx));
}

void app_container::StopApplication()
{
  app->dispose();
  // Close the channel and, transitively, the sockets
  chan->close();
}
}
