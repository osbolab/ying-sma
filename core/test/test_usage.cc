#include "actor.hh"
#include "message.hh"
#include "bytebuffer.hh"
#include "bytes.hh"


class DiscoveryActor : public Actor
{
  class Helo : public Message
  {
 public:
    std::size_t put_in(ByteBuffer& dst) const override
    {
      std::size_t written{0};
      written += Message::write_fields(dst, len);
      assert ((len -= written) > 0);
      written += Bytes.put(dst, text, text.size());
      return dst - dst_start;
    }

  protected:
    void get_fields(ReadableByteBuffer& src) override
    {
      Message::get_fields(src, len);
    }

  private:
    std::string text;
  };

  void receive(const Message& msg) override { auto resp = Helo(msg); }
};

int main(int argc, char** argv)
{
  auto sock = NativeSocket("0.0.0.0", 19997);
  sock.bind();

  auto chan = NativeChannel(sock);

  AppContainer container <

      return 0;
}
