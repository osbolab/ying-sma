#ifndef MESSAGE_H_
#define MESSAGE_H_

#include <cstdlib>
#include <cstdint>
#include <vector>

namespace sma
{

class Message {
public:
  //! Create a new message with a copy of the given data as its contents.
  Message(const std::uint8_t* data, std::size_t len);

  Message(const Message& copy);
  Message(Message&& move);

  Message& operator =(Message copy);
  Message& operator =(Message&& move);

  const std::vector<const uint8_t>& getData() const;

private:
  std::vector<const uint8_t> data;
};

}

#endif