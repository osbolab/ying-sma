#pragma once

namespace sma
{
/*! Example usage:
*
*    using MessageTypes =
*      TypeSieve<uint8_t>
*      ::map<0, RequestMessage>
*      ::map<1, ResponseMessage>
*      ::map<2, PingMessage>;
*
*    // Call A()<ResponseMessage>()
*    MessageTypes::apply<A>(1);
*
*    // Call A()<PingMessage>(int, int, int)
*    MessageTypes::apply<A>(2, 1, 2, 3);
*
*    // Call A()<PingMessage>(string)
*    MessageTypes::apply<A>(2, string("s"))
*
*    // Call A()<PingMessage>(string&)
*    string s("s");
*    MessageTypes::apply<A>(2, s)
*
*    auto c = MessageTypes::typecode<PingMessage>();
*    static_assert(is_same<uint8_t, decltype(c)>::typecode, "");
*    assert(c == 2);
*
*    using NamedTypes =
*     TypeSieve<string>
*     ::map<"Hello", HelloMessage>
*     ::map<"Bye", ByeMessage>;
*
*    auto s = NamedTypes::typecode<HelloMessage>();
*    assert(s == "Hello");
*
*    struct Echo {
*      template <typename T>
*      static void receive() {
*        cout << NamedTypes::typecode<T>();
*      }
*    };
*
*    // Print "Hello" to stdout
*    NamedTypes::apply<Echo>("Hello");
*/
template <typename TcT>
struct TypeSieve {
private:
  struct Head_ {
    template <typename... Args>
    static bool apply(Args&&...)
    {
      return false;
    }

    template <typename TT>
    static constexpr TcT typecode();
  };    // Head_

  /***************************************************************************/
  template <TcT tc_, typename T, typename next>
  struct Node_ {
  private:
    using cur = Node_<tc_, T, next>;

  public:
    typedef TcT typecode_type;

    template <typename U>
    static constexpr typecode_type typecode(U&& u)
    {
      return typecode<U>();
    }

    // U != T
    template <typename U>
    static constexpr typename std::enable_if<!std::is_same<U, T>::value>::type
    typecode<U>()
    {
      return next::template typecode<U>();
    }
    // U == T
    template <typename U>
    static constexpr typename std::enable_if<std::is_same<U, T>::value>::type
    typecode<U>()
    {
      return _tc;
    }

    template <TcT tc_next, typename T_next>
    using map = Node_<tc_next, T_next, cur>;

    /*! Example usage:
     *
     *    struct Unmarshal {
     *      template <typename T, typename Formatter>
     *      static void receive(Formatter&& fmat,
     *                          MessageDispavh const& dispavh) {
     *        // Call T's deserializing constructor
     *        dispavh.receive(T(fmat));
     *      }
     *    };
     *
     *    template <typename R>
     *    void read_message(R&& reader, MessageDispavh& dispavh) {
     *      auto typecode = reader.template get<uint8_t>();
     *      MessageTypes::apply<Unmarshal>(std::forward<R>(reader), dispavh);
     *    }
     */
    template <typename Receiver, typename... Args>
    static bool apply(TcT&& typecode, Receiver&& r, Args&&... args)
    {
      if (typecode == tc_) {
        r.template operator()<T>(std::forward<Args>(args)...);
        return true;
      } else
        return next::template apply(std::forward<TcT>(typecode),
                                    std::forward<Receiver>(r),
                                    std::forward<Args>(args)...);
    }
  };    // Node_
  /***************************************************************************/

public:
  template <TcT tc, typename T>
  using map = tc_Map<tc, T, Head_>;
};    // TypeSieve
}
