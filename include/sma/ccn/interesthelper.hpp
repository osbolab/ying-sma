#pragma once

#include <sma/ccn/interest.hpp>
#include <sma/ccn/interests.hpp>
#include <sma/ccn/contenttype.hpp>
#include <sma/ccn/detail/remoteinterestentry.hpp>

#include <sma/io/log>

#include <map>

namespace sma
{
class CcnNode;

struct MessageHeader;
struct InterestMessage;


class InterestHelper : public Interests
{
public:
  InterestHelper(CcnNode& node);
  void receive(MessageHeader&& header, InterestMessage&& msg);

  virtual void insert_new(std::vector<ContentType> types) override;

private:
  void schedule_broadcast();
  void broadcast_interests();

  CcnNode* node;
  Logger log;

  std::map<ContentType, Interest> table;
  std::map<ContentType, detail::RemoteInterestEntry> r_table;
};
}
