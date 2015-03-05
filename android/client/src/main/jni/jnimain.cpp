#include <jni.h>

#include <sma/android/jvm.hpp>

#include <cassert>


JavaVM* g_jvm;


extern "C"
{
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* jvm, void* reserved)
{
  g_jvm = jvm;

  return JNI_VERSION_1_6;
}
}

void get_env(JNIEnv*& env_out)
{
  int status = g_jvm->GetEnv((void**) &env_out, JNI_VERSION_1_6);
  assert(status == 0 && "JavaVM::GetEnv failed");
  assert(env_out != nullptr && "JNIEnv* is null after JavaVM::GetEnv");

  g_jvm->AttachCurrentThread(&env_out, 0);
}