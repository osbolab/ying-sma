// Suppress "empty struct has size 0 in C, size 1 in C++"
#pragma GCC diagnostic ignored "-Wextern-c-compat"

#include <jni.h>

#include <sma/ccn/ccnnode.hpp>
#include <sma/utility.hpp>

#include <android/log.h>

class MyClass
{
public:
  MyClass(int i) { __android_log_print(ANDROID_LOG_INFO, "JNI", "MyClass(%d)", i); }
  ~MyClass() { __android_log_print(ANDROID_LOG_INFO, "JNI", "~MyClass()"); }
};

// Remove C++ name mangling for JNI functions
extern "C" {

jstring Java_edu_asu_sma_NodeContainer_stringFromJNI(JNIEnv* env, jobject thiz)
{
  auto u = std::make_unique<MyClass>(5);

  return env->NewStringUTF("Hello from NodeContainer in C++11! Made a unique pointer :)");
}

}
