#pragma once

namespace sma
{
/*! Example usage:
*
*    using MessageTypes =
*      TypecodeSieve<uint8_t>
*      ::map<0, RequestMessage>
*      ::map<1, ResponseMessage>
*      ::map<3, PingMessage>;
*/
template <typename TypecodeT>
struct TypecodeSieve {
private:
  struct Head_ {
    template <typename... Args>
    static bool apply(Args&&...)
    {
      return false;
    }

    template <typename TT>
    static constexpr TypecodeT typecode();
  }; // Head_

  /***************************************************************************/
  template <TypecodeT tc_, typename T, typename next>
  struct Node_ {
  private:
    using cur = Node_<tc_, T, next>;

  public:
    template <TypecodeT tc_next, typename T_next>
    using map = Node_<tc_next, T_next, cur>;

    /*! Example usage:
     *
     *    struct Unmarshal {
     *      template <typename T, typename Formatter>
     *      static void receive(Formatter&& fmat,
     *                          MessageDispatch const& dispatch) {
     *        // Call T's deserializing constructor
     *        dispatch.receive(T(fmat));
     *      }
     *    };
     *
     *    template <typename R>
     *    void read_message(R&& reader, MessageDispatch& dispatch) {
     *      auto typecode = reader.template get<uint8_t>();
     *      MessageTypes::apply<Unmarshal>(std::forward<R>(reader), dispatch);
     *    }
     */
    template <typename Receiver, typename... Args>
    static bool apply(TypecodeT&& typecode, Args&&... args)
    {
      if (typecode == tc_) {
        Receiver::receive<T>(std::forward<Args>(args)...);
        return true;
      } else
        return next::template apply<Receiver>(std::forward<TypecodeT>(typecode),
                                              std::forward<Args>(args)...);
    }

    /*! Example usage:
     *
     *    PingMessage pm("Hello!");
     *
     *    template <typename W, typename M>
     *    void write_message(W&& writer, M&& msg) {
     *      writer << MessageTypes::typecode<decltype(>();
     *      writer << std::forward<M>(msg);
     *    }
     */
    template <typename U>
    static constexpr TypecodeT typecode()
    {
      return next::template typecode<U>();
    }
    template <>
    static constexpr TypecodeT typecode<T>()
    {
      return tc_;
    }
  }; // Node_
  /***************************************************************************/

public:
  template <Tc tc, typename T>
  using map = Tc_Map<tc, T, Head_>;
}; // TypecodeSieve
}
