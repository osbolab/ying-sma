#include <cstdlib>
#include <cstdint>
#include <string>
#include <iostream>
#include <utility>
#include <typeinfo>
#include <type_traits>
using std::size_t;
using std::string;
using std::cout;
using std::endl;
using std::declval;

typedef uint32_t MessageType;

static int depth = 0;


struct MessageHeader {
  MessageType type;
};

struct UntypedMessage {
  MessageHeader header;
  unsigned char const* data;
  size_t size;

  UntypedMessage(MessageHeader header, unsigned char const* data, size_t size)
    : header(header)
    , data(data)
    , size(size)
  {
    cout << "untyped message constructed" << endl;
  }
  UntypedMessage(UntypedMessage const& m)
    : header(m.header)
    , data(m.data)
    , size(m.size)
  {
    cout << "untyped message copied!" << endl;
  }
  UntypedMessage(UntypedMessage&& m)
    : header(std::move(m.header))
    , data(m.data)
    , size(m.size)
  {
    cout << "untyped message moved!" << endl;
  }
};

template <typename T>
struct TypedMessage {
  MessageHeader header;
  T body;

  template <typename M>
  TypedMessage(M&& m, T&& body)
    : header(std::forward<decltype(declval<M>().header)>(m.header))
    , body(std::forward<T>(body))
  {
  }

  operator string() { return string(body); }
};

struct MessageDispatch {
  template <typename M>
  static void receive(M&& msg)
  {
    cout << "Dispatching " << string(msg) << endl;
  }
};


namespace detail
{
struct message_types_chain_end_ {
  template <typename From, typename Target>
  static constexpr bool apply(From&& m, Target&& t)
  {
    return false;
  }
};
}


template <typename From>
struct TypeExtractor;

template <>
struct TypeExtractor<UntypedMessage> {
  static MessageType get_type(UntypedMessage const& m) { return m.header.type; }
};

template <typename From>
struct DataExtractor;

template <>
struct DataExtractor<UntypedMessage> {
  using const_data = unsigned char const*;
  using size_type = std::size_t;
  static const_data data(UntypedMessage const& m) { return m.data; }
  static size_type size(UntypedMessage const& m) { return m.size; }
};


template <typename From,
          typename ReceiverT,
          typename DexT
          = DataExtractor<typename std::remove_reference<From>::type>>
struct Unmarshaller {
  Unmarshaller(ReceiverT* receiver)
    : receiver(receiver)
  {
  }
  template <typename MessageT, typename FromT>
  void apply(FromT&& m)
  {
    auto data = DexT::data(m);
    auto size = DexT::size(m);
    // Do unformatting here!
    receiver->receive(MessageT(data));
  }

private:
  ReceiverT* receiver;
};


template <MessageType Type,
          typename MessageT,
          typename next = detail::message_types_chain_end_>
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

struct NeighborMessage;
struct ContentMessage;

using CcnMessages
    = MessageTypeSieve<0, NeighborMessage>::with<3, ContentMessage>;

int main(int argc, char** argv)
{
  UntypedMessage msg{MessageHeader{3}, nullptr, 32};

  MessageDispatch dispatch;

  Unmarshaller<UntypedMessage, MessageDispatch> um(&dispatch);
  CcnMessages::apply(msg, &um);

  return 0;
}

struct NeighborMessage {
  template <typename Reader>
  NeighborMessage(Reader* r)
  {
    cout << "NeighborMessage::phony baloney deserializer ("
         << " bytes)" << endl;
  }
  operator string() { return string("Neighbor Discovery"); }
};

struct ContentMessage {
  template <typename Reader>
  ContentMessage(Reader* r)
  {
    cout << "ContentMessage::weeee ("
         << " bytes)" << endl;
  }
  operator string() { return string("Content Metadata"); }
};
