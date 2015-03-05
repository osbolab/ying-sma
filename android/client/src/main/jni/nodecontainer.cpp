#include <sma/android/nodecontainer.hpp>

#include <sma/context.hpp>

#include <sma/linklayerimpl.hpp>
#include <sma/android/bsdinetlink.hpp>

#include <sma/neighborhelperimpl.hpp>
#include <sma/ccn/interesthelperimpl.hpp>
#include <sma/ccn/contenthelperimpl.hpp>
#include <sma/ccn/behaviorhelperimpl.hpp>

#include <sma/ccn/ccnnode.hpp>

#include <sma/io/log.hpp>
#include <sma/utility.hpp>

#include <vector>
#include <cstring>
#include <cstdint>


JNIEXPORT jboolean JNICALL
Java_edu_asu_sma_NodeContainer_create(JNIEnv* env, jobject thiz, jint id)
{
  if (sma::node != nullptr)
    return false;

  auto log = sma::Logger("NodeContainer");

  std::vector<std::unique_ptr<sma::Link>> links;
  auto inet = static_cast<sma::Link*>(new sma::BsdInetLink(env));
  links.emplace_back(inet);
  sma::linklayer = std::make_unique<sma::LinkLayerImpl>(std::move(links));

  auto node_id = sma::NodeId(std::uint16_t(id));

  sma::ctx = std::make_unique<sma::Context>(std::string(node_id),
                                            static_cast<sma::LinkLayer&>(*sma::linklayer));
  sma::node = std::make_unique<sma::CcnNode>(node_id, *sma::ctx);

  sma::neighbor_helper = std::make_unique<sma::NeighborHelperImpl>(*sma::node);
  sma::interest_helper = std::make_unique<sma::InterestHelperImpl>(*sma::node);
  sma::content_helper = std::make_unique<sma::ContentHelperImpl>(*sma::node);

  sma::linklayer->receive_to(*sma::node);

  log.d("Hello, world!");

  return true;
}

JNIEXPORT void JNICALL
Java_edu_asu_sma_NodeContainer_dispose(JNIEnv* env, jobject thiz)
{
  sma::node->stop();
  sma::linklayer->stop();
  sma::node = nullptr;
  sma::linklayer = nullptr;

  sma::Logger("NodeContainer").d("Goodbye, cruel world!");
}


namespace sma
{
std::unique_ptr<Context> ctx;
std::unique_ptr<LinkLayerImpl> linklayer;

std::unique_ptr<NeighborHelperImpl> neighbor_helper;
std::unique_ptr<InterestHelperImpl> interest_helper;
std::unique_ptr<ContentHelperImpl> content_helper;
std::unique_ptr<BehaviorHelperImpl> behavior_helper;

std::unique_ptr<CcnNode> node;
}