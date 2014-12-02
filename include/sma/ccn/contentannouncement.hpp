#pragma once

#include <sma/ccn/contentdescriptor.hpp>

#include <sma/util/serial.hpp>

namespace sma
{
struct ContentAnnouncement {
  TRIVIALLY_SERIALIZABLE(ContentAnnouncement, descriptor)

  ContentDescriptor descriptor;

  ContentAnnouncement(ContentDescriptor descriptor)
    : descriptor(std::move(descriptor))
  {
  }

  ContentAnnouncement(ContentAnnouncement&&) = default;
  ContentAnnouncement(ContentAnnouncement const&) = default;

  ContentAnnouncement& operator=(ContentAnnouncement&&) = default;
  ContentAnnouncement& operator=(ContentAnnouncement const&) = default;
};
}
