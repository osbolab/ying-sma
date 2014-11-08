
class DiscoveryActor : public Actor
{
  class Helo : public Message
  {
 public:
    std::size_t* write_fields(ByteBuffer& dst) const override
    {
      std::size_t off{0};
      off += Message::write_fields(dst, len);
      assert ((len -= off) > 0);
      dst += Bytes.put(dst, text, text.size());
      return dst - dst_start;
    }

  protected:
    std::size_t* read_fields(const ByteBuffer& csrc) override
    {
      src += Message::read_fields(src, len);
      src += Bytes.read(src, text);
      return src - src_start;
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
