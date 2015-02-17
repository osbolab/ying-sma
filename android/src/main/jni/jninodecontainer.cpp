// Suppress "empty struct has size 0 in C, size 1 in C++"
#pragma GCC diagnostic ignored "-Wextern-c-compat"

#include <jni.h>

#include <sma/ccn/ccnnode.hpp>
#include <sma/io/log.hpp>
#include <sma/utility.hpp>

// Remove C++ name mangling for JNI functions
extern "C" {

jstring Java_edu_asu_sma_NodeContainer_stringFromJNI(JNIEnv* env, jobject thiz)
{
  return env->NewStringUTF("Hello from NodeContainer in C++11!");
}

}
