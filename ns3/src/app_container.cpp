#include <sma/ns3/app_container.hpp>

#include <sma/channel.hpp>
#include <sma/message_dispatch.hpp>
#include <sma/scheduler.hpp>

#include <sma/app/application.hpp>
#include <sma/chrono>

#include <sma/bytes.hpp>
#include <sma/log>

#include <ns3/application.h>
#include <ns3/uinteger.h>

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
app_container::app_container() { LOG(DEBUG); }
app_container::app_container(Myt&& rhs) {}
app_container& app_container::operator=(Myt&& rhs) { return *this; }
app_container::~app_container() {}
// Inherited from ns3::Application; part of their lifecycle management I guess
void app_container::DoDispose() { LOG(DEBUG); }
/* c/dtor and assignment
 *****************************************************************************/

void app_container::StartApplication()
{
  // Force the clock to use the current real wall time as the beginning
  // of the simulation.
  sma::chrono::system_clock::now();

  // Construct the message chain from the bottom up
  ns3::TypeId udp_sock_factory
      = ns3::TypeId::LookupByName("ns3::UdpSocketFactory");
  sock = std::make_unique<ns3_socket>(udp_sock_factory, GetNode());
  sock->bind(socket_addr("0.0.0.0", 9999));
  chan = std::make_unique<ns3_channel>(sock.get());

  msgr = message_dispatch::new_single_threaded(chan.get());
  chan->inbox(msgr.get());

  // The scheduler is less of a chain
  sched = std::make_unique<ns3_scheduler>();

  context ctx { id, msgr.get(), sched.get() };
  app = std::make_unique<application>(std::move(ctx));
}

void app_container::StopApplication()
{
  app->dispose();
  // Close the channel and, transitively, the sockets
  chan->close();
}
}
