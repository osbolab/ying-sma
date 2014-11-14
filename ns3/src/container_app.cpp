#include <sma/ns3/container_app.hpp>

#include <sma/channel.hpp>
#include <sma/message_dispatch.hpp>
#include <sma/scheduler.hpp>

#include <sma/app/application.hpp>

#include <sma/bytes.hpp>
#include <sma/log.hpp>

#include <ns3/application.h>
#include <ns3/uinteger.h>

#include <cstdint>
#include <memory>
#include <string>


namespace sma
{
ns3::TypeId container_app::TypeId()
{
  static ns3::TypeId tid
      = ns3::TypeId("sma::container_app")
            .SetParent<ns3::Application>()
            .AddConstructor<container_app>()
            .AddAttribute("ID",
                          "Node ID",
                          ns3::UintegerValue(0),
                          ns3::MakeUintegerAccessor(&container_app::id),
                          ns3::MakeUintegerChecker<std::uint16_t>());
  return tid;
}

/******************************************************************************
 * c/dtor and assignment
 */
container_app::container_app() { LOG(DEBUG); }
container_app::container_app(Myt&& rhs) {}
container_app& container_app::operator=(Myt&& rhs) { return *this; }
container_app::~container_app() {}
// Inherited from ns3::Application; part of their lifecycle management I guess
void container_app::DoDispose() { LOG(DEBUG); }
/* c/dtor and assignment
 *****************************************************************************/

void container_app::StartApplication()
{
  // Construct the message chain from the bottom up
  ns3::TypeId udp_sock_factory
      = ns3::TypeId::LookupByName("ns3::UdpSocketFactory");
  sock = std::make_unique<ns3_socket>(udp_sock_factory, GetNode());
  sock->bind(socket_addr("0.0.0.0", 9999));

  chan = std::make_unique<ns3_channel>(sock.get());

  node::id this_node{std::uint8_t(id >> 8), std::uint8_t(id & 0xFF)};
  msgr = message_dispatch::new_single_threaded(this_node);

  chan->deliver_to(msgr.get());
  msgr->post_via(chan.get());

  // The scheduler is less of a chain
  sched = std::make_unique<ns3_scheduler>();

  context ctx { id, msgr.get(), sched.get() };
  app = std::make_unique<application>(std::move(ctx));
}

void container_app::StopApplication()
{
  app = nullptr;
  // Close the channel and, transitively, the sockets
  chan->close();
}
}
