#include "messenger.hh"

#include "message.hh"
#include "channel.hh"
#include "rws_mutex.hh"
#include "log.hh"

#include <cstdint>


namespace sma
{

void Messenger::subscribe(Message::Type t, msg_handler h)
{
  {
    writer_lock lock(mx);

    // We could use a binary search to find the insertion point, but
    // it still then requires an O(n) step to create the gap, so
    // we'd have an O(log) search and an O(n) insert.
    // Instead just do them at the same time by sliding in the back (heh)

    vec.emplace_back(std::move(t), std::move(h));
    auto i = vec.size();
    while (--i > 0 && vec[i].first < vec[i - 1].first) {
      std::swap(vec[i-1], vec[i]);
    }
  }
}

void Messenger::dispatch(const Message& msg)
{
  std::vector<msg_handler> handlers;

  {    // LOCK VEC FOR READING
    reader_lock lock(mx);

    if (vec.empty()) {
      LOG(WARNING) << "No message handlers; dropping type " << int(msg.type());
      return;
    }

    const vec_size sz = vec.size();
    const Message::Type t = msg.type();
    vec_size min = 0, max = sz - 1;
    vec_size first = sz, last = sz;

    // Multiple handlers for one message type are allowed.
    // Since they're sorted we'll find them all in a block;
    // just get the beginning and end of that block.

    // Approach from the right and find the leftmost index
    while (min <= max) {
      vec_size i = min + (max - min) / 2;
      if (vec[i].first >= t) {
        max = i - 1;
        if (vec[i].first == t)
          first = i;
      } else {
        min = i + 1;
      }
    }
    // Approach from left and find the rightmost index
    min = 0;
    max = sz - 1;
    while (min <= max) {
      vec_size i = min + (max - min) / 2;
      if (vec[i].first <= t) {
        min = i + 1;
        if (vec[i].first == t)
          last = i;
      } else {
        max = i - 1;
      }
    }

    if (first != sz && last != sz) {
      const auto count = last - first + 1;
      handlers.resize(count);
      for (vec_size i = 0; i < count; ++i) {
        handlers[i] = vec[first + i].second;
      }
    }
  }    // UNLOCK VEC

  if (!handlers.empty())
    for (auto& handler : handlers)
      handler(msg);
  else
    LOG(WARNING) << "Unhandled message type " << std::size_t{msg.type()};
}

int Messenger::send(Message builder)
{
  std::uint8_t buf[SEND_BUFFER_SIZE];
#if 0
  Message msg(std::move(builder), this_sender);
  if (msg.serialized_size() > SEND_BUFFER_SIZE)
    return -1;
  std::size_t wrote = msg.put_in(buf, SEND_BUFFER_SIZE) - buf;
  outbound->write(buf, wrote);
#endif

  return 0;
}
}
