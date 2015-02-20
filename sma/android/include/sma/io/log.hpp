#pragma once

#include <android/log.h>

#include <string>
#include <sstream>
#include <utility>
#include <cassert>

#define LOG(level) noop_logger()

struct noop_logger {
  template <typename T>
  noop_logger& operator<<(T const& t) {
    return *this;
  }
};

namespace sma
{
// clang-format off
// Eventually I'll replace this with something less, you know, completely
// horrible. Probably not though.
struct Logger {
  Logger(std::string id) : id(id) {}
  template <typename... Args> Logger const& t(Args&&... args) const;
  template <std::size_t N, typename... Args>
    Logger const& d(char const(&fmat)[N], Args&&... args) const;
  template <std::size_t N, typename... Args>
    Logger const& i(char const(&fmat)[N], Args&&... args) const;
  template <std::size_t N, typename... Args>
    Logger const& w(char const(&fmat)[N], Args&&... args) const;
  template <std::size_t N, typename... Args>
    Logger const& e(char const(&fmat)[N], Args&&... args) const;
  template <typename... Args> Logger const& f(Args&&... args) const;
private:
  std::string id;
};
// clang-format on

namespace detail
{
template <bool substitute, std::size_t N, typename... Args>
struct Printv;

template <std::size_t N>
bool substitution(char const(&fmat)[N], std::size_t i) {
  return (i+1 < N) && fmat[i] == '%' && fmat[i+1] == 'v';
}

// Process the next character
template <std::size_t N, typename... Args>
void printv(std::ostream& out, char const(&fmat)[N], std::size_t i, Args&&... args) {
  if (substitution(fmat, i))
    Printv<true, N, Args...>::format(out, fmat, i, std::forward<Args>(args)...);
  else
    Printv<false, N, Args...>::format(out, fmat, i, std::forward<Args>(args)...);
}

// Default case: no more arguments, so no more substitutions possible.
template <bool substitute, std::size_t N, typename... Args>
struct Printv {
  static void format(std::ostream& out, char const(&fmat)[N], std::size_t i) {
    while (i != N)
      out << fmat[i++];
  }
};

// Plain case: copy to the end or until encountering a substitution
template <std::size_t N, typename... Args>
struct Printv<false, N, Args...> {
  static void format(std::ostream& out, char const(&fmat)[N], std::size_t i, Args&&... args) {
    while (i < N && not substitution(fmat, i))
      out << fmat[i++];
    if (i != N) Printv<true, N, Args...>::format(out, fmat, i, std::forward<Args>(args)...);
  }
};

template <std::size_t N, typename T, typename... Args>
struct Printv<true, N, T, Args...> {
  static void format(std::ostream& out, char const(&fmat)[N], std::size_t i, T const& t, Args&&... args) {
    out << t;
    printv(out, fmat, i+2, std::forward<Args>(args)...);
  }
};

template <std::size_t N, typename... Args>
void log_v(int priority, std::string id, char const(&fmat)[N], Args&&... args) {
  std::ostringstream message;
  printv(message, fmat, 0, std::forward<Args>(args)...);
  __android_log_print(priority, id.c_str(), "%s", message.str().c_str());
}
}
/******************************************************************************
 * This is where things need to change if you switch loggers.
 */
template <typename... Args>
Logger const& Logger::t(Args&&... args) const
{
  assert(false && "Trace logging not implemented!");
  return *this;
}
template <std::size_t N, typename... Args>
Logger const& Logger::d(char const(&fmat)[N], Args&&... args) const
{
  detail::log_v(ANDROID_LOG_DEBUG, id, fmat, std::forward<Args>(args)...);
  return *this;
}
template <std::size_t N, typename... Args>
Logger const& Logger::i(char const(&fmat)[N], Args&&... args) const
{
  detail::log_v(ANDROID_LOG_INFO, id, fmat, std::forward<Args>(args)...);
  return *this;
}
template <std::size_t N, typename... Args>
Logger const& Logger::w(char const(&fmat)[N], Args&&... args) const
{
  detail::log_v(ANDROID_LOG_WARN, id, fmat, std::forward<Args>(args)...);
  return *this;
}
template <std::size_t N, typename... Args>
Logger const& Logger::e(char const(&fmat)[N], Args&&... args) const
{
  detail::log_v(ANDROID_LOG_ERROR, id, fmat, std::forward<Args>(args)...);
  return *this;
}
template <typename... Args>
Logger const& Logger::f(Args&&... args) const
{
  assert(false && "Failure logging not implemented!");
  return *this;
}
}
