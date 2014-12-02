#pragma once

#include <sma/util/ringbuffer.hpp>

namespace sma
{
namespace detail
{
  //! Represents one buffered serialized message.
  /*! These are meant to be modified in-place by claiming one from the buffer
   * and populating it.
   */
  struct MessageData {
    MessageData() = default;
    MessageData(MessageData&&) = delete;
    MessageData(MessageData const&) = delete;
    MessageData& operator=(MessageData&&) = delete;
    MessageData& operator=(MessageData const&) = delete;

    char data[1024];
    std::size_t size;
  };
}

struct MessageBuffer {
  using type = RingBuffer<detail::MessageData>;
  using write_lock = type::WriteLock;
  using read_lock = type::ReadLock;

private:
  MessageBuffer();
};
}
