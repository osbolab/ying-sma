#include <sma/ccn/ccnmessage.hpp>

namespace sma
{
  CcnMessage::CcnMessage(Type type, const_data* data, size_type size)
    : ccn_type(type), data(data), data_size(size)
  {
  }
}
