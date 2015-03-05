#pragma once

#include <jni.h>

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
extern JNIEnv* android_service_env;

extern std::multimap<std::chrono::nanoseconds, std::function<void()>> asynctaskqueue;

template <typename D>
void schedule_async_on_service_thread(D duration)
{
  assert(android_service != nullptr && "jobject has not been initialized for NativeService");
  assert(android_service_env != nullptr && "JNIEnv* has not been initialized by NativeService");
  jlong nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();

  auto cls = android_service_env->GetObjectClass(android_service);
  auto scheduleNativeAsync = android_service_env->GetMethodID(cls, "scheduleNativeAsync", "(J)V");
  assert(scheduleNativeAsync != NULL);

  android_service_env->CallVoidMethod(android_service, scheduleNativeAsync, nanos);
}
}