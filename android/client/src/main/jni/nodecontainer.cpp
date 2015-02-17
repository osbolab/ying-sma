#include <sma/android/nodecontainer.hpp>

#include <sma/android/bsdinetlink.hpp>

#include <sma/io/log.hpp>
#include <sma/utility.hpp>

#include <cstring>
#include <vector>


std::unique_ptr<sma::NodeContainer> container(nullptr);


JNIEXPORT void JNICALL
Java_edu_asu_sma_NodeContainer_create(JNIEnv* env, jobject thiz)
{
  container.reset(new sma::NodeContainer(env));
}

JNIEXPORT void JNICALL
Java_edu_asu_sma_NodeContainer_dispose(JNIEnv* env, jobject thiz)
{
  container = nullptr;
}


namespace sma
{
NodeContainer::NodeContainer(JNIEnv* env)
  : log(Logger("NodeContainer"))
{
  log.d("Hello, world!");

  std::vector<std::unique_ptr<Link>> links;
  auto inet = static_cast<Link*>(new BsdInetLink(env));
  char buf[1024];
  std::memset(buf, 0, 1024);

  assert(inet->write(buf, 1024) == 1024);
  log.d("Wrote 1024 bytes to the broadcast socket");

  links.emplace_back(inet);
  linklayer = std::make_unique<LinkLayerImpl>(std::move(links));
}

NodeContainer::~NodeContainer()
{
  log.d("Goodbye, cruel world!");
}
}
