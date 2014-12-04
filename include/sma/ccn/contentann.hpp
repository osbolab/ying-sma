#pragma once

#include <sma/ccn/contentdescriptor.hpp>

#include <sma/util/serial.hpp>

namespace sma
{
struct ContentAnn {
  TRIVIALLY_SERIALIZABLE(ContentAnn, descriptor)

  ContentDescriptor descriptor;

  ContentAnn(ContentDescriptor descriptor)
    : descriptor(std::move(descriptor))
  {
  }

  ContentAnn(ContentAnn&&) = default;
  ContentAnn(ContentAnn const&) = default;

  ContentAnn& operator=(ContentAnn&&) = default;
  ContentAnn& operator=(ContentAnn const&) = default;
};
}
