// Suppress "empty struct has size 0 in C, size 1 in C++"
#pragma GCC diagnostic ignored "-Wextern-c-compat"

#include <sma/jni/edu_asu_sma_nodecontainer.h>

#include <sma/android/nodecontainer.hpp>
#include <sma/io/log.hpp>

#include <memory>

std::unique_ptr<sma::NodeContainer> container(nullptr);


void Java_edu_asu_sma_NodeContainer_create(JNIEnv* env, jobject thiz)
{
  container.reset(new sma::NodeContainer());
}

void Java_edu_asu_sma_NodeContainer_dispose(JNIEnv* env, jobject thiz)
{
  container = nullptr;
}

jstring Java_edu_asu_sma_NodeContainer_stringFromJNI(JNIEnv* env, jobject thiz)
{
  return env->NewStringUTF("Hello from NodeContainer in C++11!");
}
