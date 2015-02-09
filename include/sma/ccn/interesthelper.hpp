#pragma once

#include <sma/helper.hpp>
#include <sma/ccn/interest.hpp>
#include <sma/ccn/ccnfwd.hpp>

#include <vector>


namespace sma
{
class InterestHelper : public Helper
{
public:
  InterestHelper(CcnNode& node);

  virtual ~InterestHelper() {}

  virtual void receive(MessageHeader header, InterestAnn announcement) = 0;

  virtual void create_local(std::vector<Interest> interests) = 0;

  virtual std::vector<Interest> local() const = 0;

  virtual bool interested_in(ContentMetadata const& metadata) const = 0;
  virtual bool know_remote(Interest const& interest) const = 0;

  virtual std::size_t announce() = 0;
};
}
