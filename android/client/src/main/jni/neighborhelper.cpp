#include <sma/android/neighborhelper.hpp>
#include <sma/android/nodecontainer.hpp>

#include <sma/neighborhelperimpl.hpp>

#include <vector>
#include <cassert>


JNIEXPORT jobject JNICALL Java_edu_asu_sma_NeighborHelper_all(JNIEnv* env, jobject thiz)
{
  assert(sma::neighbor_helper != nullptr);

  auto neighbors = sma::neighbor_helper->get();

  std::vector<std::string> ids;
  ids.reserve(neighbors.size());
  for (auto const& neighbor : neighbors)
    ids.emplace_back(neighbor.id);

  jclass list_class = env->FindClass("java/util/ArrayList");
  jobject list_obj = env->NewObject(list_class, env->GetMethodID(list_class, "<init>", "()V"));

  for (auto str : ids)
    env->CallVoidMethod(list_obj,
                        env->GetMethodID(list_class, "add", "(java/lang/Object)V"),
                        env->NewStringUTF(str.c_str()));

  return list_obj;
}
