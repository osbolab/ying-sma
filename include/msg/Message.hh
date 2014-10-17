#pragma once

#include <cstdlib>
#include <cstdint>
#include <vector>

namespace sma
{

class Message
{
public:
  typedef std::uint8_t  Type;
  typedef std::string   Tag;

  Message(const std::vector<std::uint8_t>& copy);
  Message(std::vector<std::uint8_t>&& move);

  const std::vector<std::uint8_t>& data() const;

  bool operator ==(const Message& other) const;
  bool operator !=(const Message& other) const;

protected:
  Message();
  Message(std::size_t len);

protected:
  std::vector<std::uint8_t> dataBytes;
};

class MutableMessage : public Message
{
public:
  MutableMessage();
  MutableMessage(const Message& copy);
  MutableMessage(std::size_t len);
  MutableMessage(const std::vector<std::uint8_t>& copy);
  MutableMessage(std::vector<std::uint8_t>&& move);

  std::vector<std::uint8_t>& data();

  const Message& immutableView() const;
};

}