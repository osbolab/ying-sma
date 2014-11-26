#pragma once

#include <sma/messagetype.hpp>

namespace sma
{
namespace detail
{
  struct message_sieve_chain_root_ {
    template <typename... Args>
    static constexpr bool apply(Args&&...)
    {
      return false;
    }
  };
}


template <typename From>
struct TypeExtractor;


template <MessageType Type,
          typename MessageT,
          typename next = detail::message_sieve_chain_root_>
struct MessageTypeSieve {
private:
  using cur = MessageTypeSieve<Type, MessageT, next>;

public:
  template <MessageType next_Type, typename next_MessageT>
  using with = MessageTypeSieve<next_Type, next_MessageT, cur>;

  using type = MessageT;

  template <typename FromT,
            typename ReceiverT,
            typename TexT
            = TypeExtractor<typename std::remove_reference<FromT>::type>>
  static bool apply(FromT&& m, ReceiverT* receiver)
  {
    if (TexT::get_type(m) == Type) {
      receiver->template apply<MessageT>(std::forward<FromT>(m));
      return true;
    } else
      return next::apply(std::forward<FromT>(m), receiver);
  }
};
}
