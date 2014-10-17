#include "msg/Message.hh"


namespace sma
{

Message::Message() {}
Message::Message(std::size_t len) : dataBytes(len) {}
Message::Message(const std::vector<std::uint8_t>& copy) : dataBytes(copy) {}
Message::Message(std::vector<std::uint8_t>&& move) : dataBytes(std::move(move)) {}

bool Message::operator ==(const Message& other) const { return dataBytes == other.dataBytes; }
bool Message::operator !=(const Message& other) const { return !(*this == other); }

const std::vector<std::uint8_t>& Message::data() const { return dataBytes; }


MutableMessage::MutableMessage() {}
MutableMessage::MutableMessage(const Message& copy) : Message(copy.data()) {}
MutableMessage::MutableMessage(std::size_t len) : Message(len) {}
MutableMessage::MutableMessage(const std::vector<std::uint8_t>& copy) : Message(copy) {}
MutableMessage::MutableMessage(std::vector<std::uint8_t>&& move) : Message(std::move(move)) {}

std::vector<std::uint8_t>& MutableMessage::data() { return dataBytes; }

const Message& MutableMessage::immutableView() const { return *this; }

}