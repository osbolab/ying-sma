#include <sma/stats.hpp>

#include <sma/io/log>


namespace sma
{
namespace stats
{
  ///
  /// Nodes
  ///
  std::vector<NodeId> all_nodes;

  ///
  /// Interests
  ///
  std::unordered_map<ContentType, time_point> Ints::sent;
  std::unordered_map<ContentType, std::vector<NodeId>> Ints::received;

  void Ints::on_sent(NodeId sender, Interest interest)
  {
    auto it = sent.find(interest.type);
    if (it == sent.end()) {
      sent.emplace(interest.type, clock::now());
      LOG(INFO) << "Interest sent, " << sender << ", " << interest.type;
    }
  }

  void Ints::on_received(NodeId recipient, Interest interest)
  {
    auto& recps = received[interest.type];
    for (auto& recp : recps)
      if (recp == recipient)
        return;
    recps.push_back(recipient);

    auto sent_time = sent[interest.type];

    LOG(INFO) << "Interest received, " << recipient << ", " << interest.type
              << ", "
              << std::chrono::duration_cast<std::chrono::milliseconds>(
                     clock::now() - sent_time).count() << ", "
              << (double(recps.size()) / all_nodes.size());
  }


  ///
  /// Metadata
  ///
  std::unordered_map<Hash, time_point> Meta::sent;
  std::unordered_map<Hash, std::vector<NodeId>> Meta::received;

  void Meta::on_sent(NodeId sender, ContentMetadata meta)
  {
    auto it = sent.find(meta.hash);
    if (it == sent.end()) {
      sent.emplace(meta.hash, clock::now());
      LOG(INFO) << "Metadata sent, " << sender << ", " << meta.hash;
    }
  }

  void Meta::on_received(NodeId recp, ContentMetadata meta)
  {
    auto& recps = received[meta.hash];
    for (auto& r : recps)
      if (r == recp)
        return;
    recps.push_back(recp);

    auto sent_time = sent[meta.hash];

    LOG(INFO) << "Metadata received, " << recp << ", " << meta.types[0] << ", "
              << std::chrono::duration_cast<std::chrono::milliseconds>(
                     clock::now() - sent_time).count();
  }
}
}
