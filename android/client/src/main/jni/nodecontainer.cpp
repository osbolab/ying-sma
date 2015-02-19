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

#include <cstring>
#include <vector>


JNIEXPORT void JNICALL
Java_edu_asu_sma_NodeContainer_create(JNIEnv* env, jobject thiz)
{
  auto log = sma::Logger("NodeContainer");
  log.d("Hello, world!");

  std::vector<std::unique_ptr<sma::Link>> links;
  auto inet = static_cast<sma::Link*>(new sma::BsdInetLink(env));
  char buf[1024];
  std::memset(buf, 0, 1024);

  assert(inet->write(buf, 1024) == 1024);
  log.d("Wrote 1024 bytes to the broadcast socket");

  links.emplace_back(inet);
  sma::linklayer = std::make_unique<sma::LinkLayerImpl>(std::move(links));
}

JNIEXPORT void JNICALL
Java_edu_asu_sma_NodeContainer_dispose(JNIEnv* env, jobject thiz)
{
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
