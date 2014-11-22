#include <sma/serialize/datareader.hpp>
#include <sma/serialize/datawriter.hpp>

namespace sma
{
template <typename Formatter>
class ObjectData : public DataReader<Formatter>, public DataWriter<Formatter>
{
public:
  DataReader(Formatter formatter)
    : f(std::move(formatter))
  {
  }
};
}
