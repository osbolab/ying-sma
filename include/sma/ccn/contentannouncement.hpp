#pragma once

#include <sma/ccn/contentinfo.hpp>

#include <sma/util/serial.hpp>

namespace sma
{
struct ContentAnnouncement {
  TRIVIALLY_SERIALIZABLE(ContentAnnouncement, info)

  ContentInfo info;

  ContentAnnouncement(ContentInfo info)
    : info(std::move(info))
  {
  }

  ContentAnnouncement(ContentAnnouncement&&) = default;
  ContentAnnouncement(ContentAnnouncement const&) = default;

  ContentAnnouncement& operator=(ContentAnnouncement&&) = default;
  ContentAnnouncement& operator=(ContentAnnouncement const&) = default;
};
}
