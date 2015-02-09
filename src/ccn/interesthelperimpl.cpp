#include <sma/ccn/interesthelperimpl.hpp>
#include <sma/ccn/interestann.hpp>

#include <sma/ccn/ccnnode.hpp>

#include <sma/context.hpp>
#include <sma/async.hpp>

#include <chrono>
#include <limits>
#include <sstream>
#include <iomanip>

using namespace std::literals::chrono_literals;

namespace sma
{
InterestHelperImpl::InterestHelperImpl(CcnNode& node)
  : InterestHelper(node)
{
}

void InterestHelperImpl::receive(MessageHeader header, InterestAnn msg)
{
  // Ignore loopback
  if (msg.interested_node == node.id)
    return;

  log.t("<-- %v interests from n(%v) via n(%v)",
        msg.interests.size(),
        msg.interested_node,
        header.sender);

  // Cull any that we aren't already broadcasting with a closer distance.
  // The result is that our neighbors always see the shortest path we know of.
  for (auto it = msg.interests.begin(); it != msg.interests.end();) {
    auto& entry = *it;
    if (!learn_remote(entry.interest))
      // Only forward interests we learned something new about
      it = msg.interests.erase(it);
    else
      ++it;
  }
}

void InterestHelperImpl::learn_remote(Interest const& interest)
{
  for (auto& i : interests)
    if (i.type == interest.type) {
      i.update_with(interest);
      return;
    }
  interests.emplace_front(
      interest.type, interest.ttl<std::chrono::milliseconds>(), interest.hops);
}

std::vector<Interest> InterestHelperImpl::local() const
{
  std::vector<Interest> interests;
  for (auto& i : interests)
    if (i.local())
      interests.push_back(i.first);
  return interests;
}

bool InterestHelperImpl::interested_in(ContentMetadata const& metadata) const
{
  for (auto& i : interests)
    if (i.local())
      for (auto& type : metadata.types)
        if (i.type == type)
          return true;
}

bool InterestHelperImpl::know_remote(Interest const& interest) const
{
  for (auto& i : interests)
    if (i.remote() && i.type == interest.type)
      return true;
}

void InterestHelperImpl::create_local(std::vector<ContentType> types)
{
  for (auto& type : types) {
    for (auto it = interests.begin(); it != interests.end(); ++it)
      if (it->type == type) {
        interests.erase(it);
        break;
      }
    interests.emplace_front(type, 10s);
  }
}

std::size_t InterestHelperImpl::announce()
{
  // Max size of one announcement message to be filled with interests
  std::size_t const max_size = 1024;
  // Serialize the interests into a contiguous block of stack to
  // a) avoid fragmenting allocations and b) provide a ready structure to
  // write the serialized interests into a message.
  std::uint8_t data_buf[max_size * 2];
  // Mark the start of the current interest's buffer space
  std::uint8_t* data = &data_buf;
  // Total serialized length
  std::size_t size = 0;
  std::size_t count = 0;

  while (count < interests.size()) {
    auto& i = interests.begin();
    i.elapse_ttl();
    if (i.expired()) {
      interests.pop_front();
      continue;
    }

    BinaryOutput out(data, sizeof(data_buf) - size);
    out << i;
    auto const wrote = out.size();
    // Don't overrun the message
    if (size + wrote > max_size)
      break;

    size += wrote;
    data += size;
    ++count;
    // Swap the interest to the end of the deque so others might be sent.
    interests.push_back(std::move(i));
    interests.pop_front();

    if (size == max_size)
      break;
  }

  if (count != 0) {
    // The Interest Announcement message just dumps the buffer into the
    // message data and reads it back out at the other end.
    InterestAnn msg(count, data, size);
    log.t("--> announce %v interests (%v bytes)", count, size);
    node.post(msg);
  }

  return count;
}
}
