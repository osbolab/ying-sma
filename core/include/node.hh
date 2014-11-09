#pragma once

                                 #include <cstdint>


namespace sma
{

  class Node
  {
    public:
      struct Id
      {
        std::uint8_t data[2];
        static const std::size_t size;
      };

      virtual ~Node() = 0;
  };

  const std::size_t Node::Id::size = sizeof(Node::Id::data);
  inline Node::~Node() {}
}
