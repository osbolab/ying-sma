#pragma once

#include <sma/io/detail/logimpl.hpp>

namespace sma
{
namespace detail
{
  using log_impl_type = el::Logger;
}

// clang-format off
  // Eventually I'll replace this with something less, you know, completely
  // horrible.
  struct Logger {
    Logger(std::string id);
    template <typename... Args> Logger const& t(Args&&... args) const;
    template <typename... Args> Logger const& d(Args&&... args) const;
    template <typename... Args> Logger const& i(Args&&... args) const;
    template <typename... Args> Logger const& w(Args&&... args) const;
    template <typename... Args> Logger const& e(Args&&... args) const;
    template <typename... Args> Logger const& f(Args&&... args) const;
  private:
    detail::log_impl_type* impl;
  };
// clang-format on

/******************************************************************************
 * This is where things need to change if you switch loggers.
 */
template <typename... Args>
Logger const& Logger::t(Args&&... args) const
{
  impl->trace(std::forward<Args>(args)...);
  return *this;
}
template <typename... Args>
Logger const& Logger::d(Args&&... args) const
{
  impl->debug(std::forward<Args>(args)...);
  return *this;
}
template <typename... Args>
Logger const& Logger::i(Args&&... args) const
{
  impl->info(std::forward<Args>(args)...);
  return *this;
}
template <typename... Args>
Logger const& Logger::w(Args&&... args) const
{
  impl->warn(std::forward<Args>(args)...);
  return *this;
}
template <typename... Args>
Logger const& Logger::e(Args&&... args) const
{
  impl->error(std::forward<Args>(args)...);
  return *this;
}
template <typename... Args>
Logger const& Logger::f(Args&&... args) const
{
  impl->fatal(std::forward<Args>(args)...);
  return *this;
}
}
