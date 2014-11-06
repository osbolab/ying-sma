#pragma once

#include "messenger.hh"
#include "message.hh"

#include <iostream>
#include <cstdint>


namespace sma
{

TEST(Messenger, movable_callback)
{
  auto mgr = Messenger();
  mgr.subscribe(Message::Type{1}, [](const Message& msg) {
    std::cout << msg.cbody() << std::endl;
    return;
  });

  std::uint8_t body[]{'H', 'e', 'l', 'l', 'o', 0};

  mgr.dispatch(Message(Message::Type{1},
                       Message::Recipient{0, 0},
                       std::vector<Recipient>(),
                       &body,
                       sizeof body));
  ASSERT_EQ(1, 0);
}
}
