#pragma once

#include <cstdint>

namespace sma
{
class CcnMessage final
{
public:
  using Type = std::uint8_t;
  using const_data = std::uint8_t const;
  using size_type = std::size_t;

  CcnMessage(Type type, const_data* data, size_type size);

  template <typename Reader>
  CcnMessage(Reader* r);

  template <typename Writer>
  void write_fields(Writer* w);

  Type type() const { return ccn_type; }
  const_data* cdata() const { return data; }
  size_type size() const { return data_size; }

private:
  Type ccn_type;
  const_data* data;
  size_type data_size;
};

template <typename Reader>
CcnMessage::CcnMessage(Reader* r)
{
  *r >> ccn_type;
  *r >> data_size;
  r->read(data, data_size);
}

template <typename Writer>
void CcnMessage::write_fields(Writer* w)
{
  *w << ccn_type;
  *w << data_size;
  w->write(data, data_size);
}
}
