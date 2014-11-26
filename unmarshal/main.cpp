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

typedef uint32_t TypeCode;

static int depth = 0;


struct MessageHeader {
  TypeCode type;
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
  static TypeCode get(UntypedMessage const& m) { return m.header.type; }
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

template <typename Target>
struct TargetDelegator;

template <>
struct TargetDelegator<MessageDispatch> {
  template <typename M>
  static void delegate(M&& m, MessageDispatch* target)
  {
    target->receive(std::forward<M>(m));
  }
};


template <typename Types>
struct Unmarshaller {
  template <typename From, typename Target>
  static bool delegate(From&& m, Target* t)
  {
    return Types::apply(std::forward<From>(m), t);
  }
};

template <TypeCode Tc,
          typename As,
          typename prev = detail::message_types_chain_end_>
struct MessageTypes {
private:
  using cur = MessageTypes<Tc, As, prev>;

public:
  template <TypeCode nTc, typename nAs>
  using with = MessageTypes<nTc, nAs, cur>;

  using type = As;

  template <typename From>
  using unmarshal = Unmarshaller<cur>;

private:
  template <TypeCode, typename, typename>
  friend struct MessageTypes;

  template <typename>
  friend struct Unmarshaller;

  template <
      typename From,
      typename Target,
      typename Tdel = TargetDelegator<typename std::remove_reference<Target>::type>,
      typename Tex = TypeExtractor<typename std::remove_reference<From>::type>,
      typename Dex = DataExtractor<typename std::remove_reference<From>::type>>
  static bool apply(From&& m, Target* t)
  {
    if (Tex::get(m) == Tc) {
      auto data = Dex::data(m);
      auto size = Dex::size(m);
      // Do unformatting here!
      Tdel::delegate(As(data), t);
      return true;
    } else
      return prev::apply(std::forward<From>(m), t);
  }
};

struct NeighborMessage;
struct ContentMessage;

using CcnMessages = MessageTypes<0, NeighborMessage>::with<3, ContentMessage>;

int main(int argc, char** argv)
{
  UntypedMessage msg{MessageHeader{3}, nullptr, 32};

  MessageDispatch dispatch;

  using um = CcnMessages::unmarshal<UntypedMessage>;
  bool b = um::delegate(msg, &msg);
  cout << endl;
  b &= um::delegate(UntypedMessage{MessageHeader{0}, nullptr, 60}, &dispatch);
  cout <<endl;
  cout << "Delegate returned " << (b ? "true" : "false") << endl;

  return 0;
}

struct NeighborMessage {
  template <typename Reader>
  NeighborMessage(Reader* r)
  {
    cout << "NeighborMessage::phony baloney deserializer (" << " bytes)"
         << endl;
  }
  operator string() { return string("Neighbor Discovery"); }
};

struct ContentMessage {
  template <typename Reader>
  ContentMessage(Reader* r)
  {
    cout << "ContentMessage::weeee (" << " bytes)" << endl;
  }
  operator string() { return string("Content Metadata"); }
};
}
#endif
