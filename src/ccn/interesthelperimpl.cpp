#include <sma/ccn/interesthelperimpl.hpp>
#include <sma/ccn/interestann.hpp>

#include <sma/ccn/ccnnode.hpp>

#include <sma/context.hpp>
#include <sma/async.hpp>

#include <sma/util/binaryformat.hpp>

#include <sma/stats.hpp>

#include <chrono>
#include <limits>
#include <sstream>
#include <iomanip>

using namespace std::literals::chrono_literals;

namespace sma
{
InterestHelperImpl::InterestHelperImpl(CcnNode& node)
  : InterestHelper(node)
  , next_announce_time(clock::now())
  , to_announce(0)
{
}

void InterestHelperImpl::receive(MessageHeader header, InterestAnn msg)
{
  std::vector<Interest> interests;
  interests.reserve(msg.count);
  Reader<BinaryInput> reader(msg.data, msg.size);
  for (std::size_t i = 0; i < msg.count; ++i)
    interests.emplace_back(reader);

  for (auto& interest : interests) {
    stats::Ints::on_received(node.id, interest);
    ++interest.hops;
    learn_remote(interest);
  }
}

void InterestHelperImpl::create_local(ContentType type)
{
  std::size_t idx = 0;
  for (auto it = interests.begin(); it != interests.end(); ++it, ++idx) {
    if (it->type == type) {
      if (it->local())
        return;

      interests.erase(it);
      // If we replaced a remote interest that was going to be announced then
      // don't count it as another interest to announce.
      if (idx < to_announce)
        --to_announce;

      break;
    }
  }

  interests.emplace_front(type, 10s);
  ++to_announce;
}

void InterestHelperImpl::create_local(std::vector<ContentType> types)
{
  for (auto& type : types)
    create_local(type);
}

void InterestHelperImpl::clear_local()
{
  auto it = interests.begin();
  while (it != interests.end())
    if (it->local())
      it = interests.erase(it);
    else
      ++it;
}

std::vector<Interest> InterestHelperImpl::all() const
{
  std::vector<Interest> all;
  all.reserve(interests.size());
  for (auto& i : interests)
    all.emplace_back(i);
  return all;
}

std::vector<Interest> InterestHelperImpl::local() const
{
  std::vector<Interest> locals;
  for (auto& i : interests)
    if (i.local())
      locals.push_back(i);
  return locals;
}

std::vector<Interest> InterestHelperImpl::remote() const
{
  std::vector<Interest> remotes;
  for (auto& i : interests)
    if (i.remote())
      remotes.push_back(i);
  return remotes;
}

bool InterestHelperImpl::interested_in(ContentMetadata const& metadata) const
{
  for (auto& i : interests)
    if (i.local())
      for (auto& type : metadata.types)
        if (i.type == type)
          return true;
  return false;
}

void InterestHelperImpl::learn_remote(Interest const& interest)
{
  std::size_t idx = 0;
  for (auto it = interests.begin(); it != interests.end(); ++it, ++idx)
    if (it->type == interest.type) {
      if (it->update_with(interest)) {
        log.d("Promoted remote interest in '%v' (%v hops)",
              interest.type,
              std::uint32_t(interest.hops));
        auto i = std::move(*it);
        interests.erase(it);
        interests.push_front(std::move(i));
        // Check if we promoted an already-announced element
        if (idx >= to_announce)
          ++to_announce;
      }
      return;
    }

  log.d("Discovered remote interest in '%v' (%v hops)",
        interest.type,
        std::uint32_t(interest.hops));
  interests.emplace_front(
      interest.type, interest.ttl<std::chrono::milliseconds>(), interest.hops);
  ++to_announce;
}

bool InterestHelperImpl::know_remote(ContentType const& type) const
{
  for (auto& i : interests)
    if (i.remote() && i.type == type)
      return true;
  return false;
}

bool InterestHelperImpl::contains_any(
    std::vector<ContentType> const& types) const
{
  for (auto& i : interests)
    for (auto& type : types)
      if (i.type == type)
        return true;
  return false;
}

std::size_t InterestHelperImpl::announce()
{
  if (clock::now() < next_announce_time)
    return 0;

  // Max size of one announcement message to be filled with interests
  std::size_t const max_size = 1024;
  // Serialize the interests into a contiguous block of stack to
  // a) avoid fragmenting allocations and b) provide a ready structure to
  // write the serialized interests into a message.
  std::uint8_t data_buf[max_size * 2];
  // Mark the start of the current interest's buffer space
  std::uint8_t* data = data_buf;
  // Total serialized length
  std::size_t size = 0;
  std::size_t count = 0;

  while (count < interests.size()) {
    auto& i = interests.front();
    i.elapse_ttl();
    if (i.expired()) {
      interests.pop_front();
      --to_announce;
      continue;
    }

    stats::Ints::on_sent(node.id, i);

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
    --to_announce;

    if (size == max_size)
      break;
  }

  if (to_announce == 0) {
    to_announce = interests.size();
    next_announce_time = clock::now() + 2s;
  }

  if (count != 0) {
    // The Interest Announcement message just dumps the buffer into the
    // message data and reads it back out at the other end.
    InterestAnn msg(count, data_buf, size);
    log.t("--> announce %v interests (%v bytes)", count, size);
    node.post(msg);
  }

  return count;
}
}
