#include <string.h>
#include <jni.h>

#include <memory>

// Remove C++ name mangling for JNI functions
extern "C" {

jstring Java_edu_asu_sma_NodeContainer_stringFromJNI(JNIEnv* env, jobject thiz)
{
  auto u = std::unique_ptr<int>();

  return env->NewStringUTF("Hello from NodeContainer in C++11! Made a unique pointer :)");
}

}
