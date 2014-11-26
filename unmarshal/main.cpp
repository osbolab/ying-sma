#include <cstdlib>
#include <cstdint>
#include <string>
#include <iostream>
#include <utility>
#include <typeinfo>
using std::size_t;
using std::string;
using std::cout;
using std::endl;
using std::declval;

typedef uint32_t TypeCode;

static int depth = 0;


struct MessageHeader {
  TypeCode type;
  string sender;
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

template <typename UnmarshalAs>
struct unmarshal_functor {
};

template <typename From, template <TypeCode> class As>
struct unmarshaller {
  template <TypeCode Tc>
  using typed_message = TypedMessage<typename As<Tc>::type>;
  template <TypeCode Tc>
  using message_body = typename As<Tc>::type;

  unmarshaller(From const& from)
    : from(from)
  {
  }
  unmarshaller(From&& from)
    : from(std::move(from))
  {
  }

  template <TypeCode Tc>
  typed_message<Tc> apply()
  {
    message_body<Tc> body(from.data, from.size);
    return typed_message<Tc>(std::forward<From>(from), std::move(body));
  }

private:
  From from;
};

namespace detail
{
struct message_types_chain_end_ {
  static bool apply(TypeCode tc)
  {
    cout << " X" << endl;
    return false;
  }
};
}

template <typename Types>
struct Unmarshaller {
  template <typename From, typename Target>
  static bool delegate(From&& m, Target&& t)
  {
    return Types::apply(std::forward<From>(m), std::forward<Target>(t));
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
  template <typename>
  friend struct Unmarshaller;

  template <typename From, typename Target>
  static bool apply(From&& m, Target&& t)
  {
    if (m.type == Tc) {
      t.receive(m);
    } else
      return prev::apply(std::forward<From>(m), std::forward<Target>(t));
  }
};

struct NeighborMessage;
struct ContentMessage;
struct MessageDispatch;

using CcnMessages = MessageTypes<0, string>::with<3, string>::with<5, int>;

int main(int argc, char** argv)
{
  cout << sizeof(MessageTypes) << " x3 = " << sizeof(CcnMessages) << endl;

  using um = CcnMessages::unmarshal<string>;
  bool b = um::delegate(true, true);
  cout << (b ? "true" : "false") << endl;

  return 0;
}

struct MessageDispatch {
  template <typename M>
  static void receive(M&& msg)
  {
    cout << string(std::forward<M>(msg)) << endl;
  }
};

struct NeighborMessage {
  NeighborMessage(unsigned char const* data, size_t size)
  {
    cout << "NeighborMessage::phony baloney deserializer (" << size << " bytes)"
         << endl;
  }
  operator string() { return string("Neighbor Discovery"); }
};

struct ContentMessage {
  ContentMessage(unsigned char const* data, size_t size)
  {
    cout << "ContentMessage::weeee (" << size << " bytes)" << endl;
  }
  operator string() { return string("Content Metadata"); }
};

#if 0
int main(int argc, char** argv) {
  if (argc < 2) return 1;

  typedef UntypedMessage message;
  typedef unmarshaller<message, CcnMessage> unmarshal;

  for (size_t i = 1; i < argc; ++i) {
    TypeCode tc(std::atoi(argv[i]));
    switch (tc) {
      case 0: {
        message msg{MessageHeader{string("untyped-msg")}, nullptr, 32};
        unmarshal instance(msg);
        MessageDispatch::receive(instance.apply<0>());
        break;
      }

      case 1: {
        message msg{MessageHeader{string("untyped-msg")}, nullptr, 32};
        unmarshal instance(std::move(msg));
        MessageDispatch::receive(instance.apply<1>());
        break;
      }

      default:
        cout << "bad name at position " << i << ": " << std::to_string(tc)
             << endl;
    }
  }

  return 0;
}
#endif
