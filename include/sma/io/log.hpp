#pragma once

#include <android/log.h>

#include <string>


namespace sma
{
// clang-format off
  // Eventually I'll replace this with something less, you know, completely
  // horrible. Probably not though.
  struct Logger {
    Logger(std::string id) : id(id) {}
    template <typename... Args> Logger const& t(Args&&... args) const;
    template <typename... Args> Logger const& d(Args&&... args) const;
    template <typename... Args> Logger const& i(Args&&... args) const;
    template <typename... Args> Logger const& w(Args&&... args) const;
    template <typename... Args> Logger const& e(Args&&... args) const;
    template <typename... Args> Logger const& f(Args&&... args) const;
  private:
    std::string id;
  };
// clang-format on

/******************************************************************************
 * This is where things need to change if you switch loggers.
 */
template <typename... Args>
Logger const& Logger::t(Args&&... args) const
{
  return *this;
}
template <typename... Args>
Logger const& Logger::d(Args&&... args) const
{
  return *this;
}
template <typename... Args>
Logger const& Logger::i(Args&&... args) const
{
  return *this;
}
template <typename... Args>
Logger const& Logger::w(Args&&... args) const
{
  return *this;
}
template <typename... Args>
Logger const& Logger::e(Args&&... args) const
{
  return *this;
}
template <typename... Args>
Logger const& Logger::f(Args&&... args) const
{
  return *this;
}
}
