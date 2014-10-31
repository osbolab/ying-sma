#pragma once

#include <cstdlib>
#include <cstdint>
#include <vector>
#include <memory>
#include <cstddef>


namespace sma
{

class Messenger;

class Message
{
public:
  typedef std::uint8_t Type;
  typedef std::string Tag;

  class Builder
  {
    friend class Messenger;

  private:
    Builder(std::shared_ptr<const Messenger> messenger);

    std::shared_ptr<const Messenger> messenger;
  };

  Message();
  Message(const std::vector<std::uint8_t>& copy);
  Message(std::vector<std::uint8_t>&& move);

  std::vector<std::uint8_t>& data();
  const std::vector<std::uint8_t>& cdata() const;

  bool operator==(const Message& other) const;
  bool operator!=(const Message& other) const;

protected:
  Message(std::size_t len);

protected:
  std::vector<std::uint8_t> dataBytes;
};
}
