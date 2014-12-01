#pragma once

#include <sma/ccn/interests.hpp>
#include <sma/ccn/contenttype.hpp>
#include <sma/ccn/interestrank.hpp>
#include <sma/ccn/remoteinterest.hpp>

#include <sma/io/log>

#include <map>

namespace sma
{
class CcnNode;

struct ContentInfo;
struct ContentType;

struct MessageHeader;
struct InterestMessage;

class InterestHelper : public Interests
{
public:
  InterestHelper(CcnNode& node);
  void receive(MessageHeader header, InterestMessage msg);

  virtual void insert_new(std::vector<ContentType> types) override;

  virtual bool interested_in(ContentInfo const& info) const override;
  virtual bool know_remote(ContentType const& type) const override;

private:
  void schedule_announcement(std::chrono::milliseconds delay
                             = std::chrono::milliseconds(5000));
  void announce();

  /*! \return true if something new was learned about the interest. */
  bool learn_remote_interest(Interest const& interest);

  void log_interest_table();

  CcnNode* node;
  Logger log;

  std::map<ContentType, InterestRank> table;
  std::map<ContentType, RemoteInterest> r_table;
};
}
