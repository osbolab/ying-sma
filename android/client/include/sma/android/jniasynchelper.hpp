#pragma once

#include <jni.h>

#include <sma/android/jvm.hpp>

#include <cassert>
#include <chrono>

#include <map>
#include <chrono>
#include <functional>


extern "C" {
JNIEXPORT void JNICALL Java_edu_asu_sma_client_NativeService_captureServicePointer(JNIEnv*, jobject);
JNIEXPORT void JNICALL Java_edu_asu_sma_client_NativeService_deleteServicePointer(JNIEnv*, jobject);
JNIEXPORT void JNICALL Java_edu_asu_sma_client_NativeService_runNativeAsyncTask(JNIEnv*, jobject);
}

namespace sma
{
extern jobject android_service;

extern std::multimap<std::chrono::system_clock::time_point, std::function<void()>> asynctaskqueue;

template <typename D>
void schedule_async_on_service_thread(D duration)
{
  assert(android_service != nullptr && "jobject has not been initialized for NativeService");
  jlong nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();

  JNIEnv* env = nullptr;
  get_env(env);

  auto cls = env->GetObjectClass(android_service);
  auto scheduleNativeAsync = env->GetMethodID(cls, "scheduleNativeAsync", "(J)V");
  assert(scheduleNativeAsync != NULL);

  env->CallVoidMethod(android_service, scheduleNativeAsync, nanos);
}
}