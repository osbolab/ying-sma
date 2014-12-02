#pragma once

#include <sma/helper.hpp>
#include <sma/ccn/ccnfwd.hpp>

#include <vector>

namespace sma
{
class InterestHelper : public Helper
{
public:
  InterestHelper(CcnNode& node);

  virtual ~InterestHelper() {}

  virtual void receive(MessageHeader header, InterestAnnouncement msg) = 0;

  virtual void insert_new(std::vector<ContentType> types) = 0;

  virtual bool interested_in(ContentDescriptor const& descriptor) const = 0;
  virtual bool know_remote(ContentType const& type) const = 0;
};
}
