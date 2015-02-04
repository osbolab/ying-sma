#include <sma/ccn/interesthelperimpl.hpp>
#include <sma/ccn/interestann.hpp>

#include <sma/ccn/ccnnode.hpp>

#include <sma/context.hpp>
#include <sma/async.hpp>

#include <chrono>
#include <limits>
#include <sstream>
#include <iomanip>

using namespace std::literals::chrono_literals;

namespace sma
{
void log_interest_table(Logger& log,
                        std::unordered_map<ContentType, RemoteInterest>& rit)
{
  log.d(" remote interest table");
  log.d("| content | age (ms) |");
  log.d("| ------- | -------- |");
  std::stringstream ss;
  for (auto it = rit.begin(); it != rit.end(); ++it) {
    ss.str("");
    auto age_ms = it->second.template age<std::chrono::milliseconds>().count();
    ss << "| " << std::left << std::setw(7) << std::string(it->first) << " | ";
    if (age_ms != 0)
      ss << std::left << std::setw(8) << std::to_string(age_ms) << " |";
    else
      ss << std::left << std::setw(8) << "   *"
         << " |";
    log.d(ss.str());
  }
  log.d("");
}

InterestHelperImpl::InterestHelperImpl(CcnNode& node)
  : InterestHelper(node)
{
  prune_remote();
}

void InterestHelperImpl::receive(MessageHeader header, InterestAnn msg)
{
  // Ignore loopback
  if (msg.interested_node == node.id)
    return;

  log.t("<-- %v interests from n(%v) via n(%v)",
        msg.interests.size(),
        msg.interested_node,
        header.sender);

  // Cull any that we aren't already broadcasting with a closer distance.
  // The result is that our neighbors always see the shortest path we know of.
  for (auto it = msg.interests.begin(); it != msg.interests.end();) {
    auto& entry = *it;
    if (!learn_remote(entry.interest))
      // Only forward interests we learned something new about
      it = msg.interests.erase(it);
    else
      ++it;
  }

  if (!msg.interests.empty()) {
    log.t("--> forward %v interests", msg.interests.size());
    node.post(msg);
  }
}

bool InterestHelperImpl::learn_remote(Interest const& interest)
{
  auto try_add = rit.emplace(interest, RemoteInterest());
  if (try_add.second)
    return true;
  auto& existing = try_add.first->second;
  return existing.update();
}

bool InterestHelperImpl::interested_in(ContentMetadata const& metadata) const
{
  return lit.find(Interest(metadata)) != lit.end();
}

bool InterestHelperImpl::know_remote(Interest const& interest) const
{
  return rit.find(interest) != rit.end();
}

void InterestHelperImpl::create_local(std::vector<Interest> types)
{
  for (auto& t : types)
    lit.emplace(t, InterestRank());
}

void InterestHelperImpl::announce()
{
  InterestAnn msg(node.id);

  // Our interests are 0 hops from us... the receiver will account for our
  // link to them.
  if (!lit.empty()) {
    msg.interests.reserve(lit.size());
    for (auto const& entry : lit)
      msg.interests.emplace_back(entry.first, true);
  }
  // Add as many remote interests to our message as we can.
  // FIXME: We need some real logic here.
  std::size_t const nmax = 255;
  if (lit.size() < nmax) {
    auto nfwds = nmax - lit.size();
    for (auto it = rit.cbegin(); nfwds-- > 0 && it != rit.cend(); ++it)
      msg.interests.emplace_back(it->first, false);
  }

  if (!msg.interests.empty()) {
    log.t("--> announce %v interests", msg.interests.size());
    node.post(msg);
  }
}

void InterestHelperImpl::prune_remote()
{
  auto it = rit.begin();
  while (it != rit.end()) {
    if (it->second.older_than(60s))
      it = rit.erase(it);
    else
      ++it;
  }

  asynctask(std::bind(&InterestHelperImpl::prune_remote, this)).do_in(60s);
}
}
