#include <sma/android/jniasynchelper.hpp>
#include <sma/async.hpp>

#include <sma/io/log.hpp>


namespace sma
{
jobject android_service = nullptr;
std::multimap<std::chrono::system_clock::time_point, std::function<void()>> asynctaskqueue;

static Logger log("NativeService");
}


JNIEXPORT void JNICALL Java_edu_asu_sma_client_NativeService_captureServicePointer(JNIEnv* env, jobject thiz)
{
  sma::android_service = env->NewGlobalRef(thiz);
  sma::log.d("Captured NativeService*");
}

JNIEXPORT void JNICALL Java_edu_asu_sma_client_NativeService_deleteServicePointer(JNIEnv* env, jobject thiz)
{
  sma::Async::purge();
  env->DeleteGlobalRef(sma::android_service);
}

JNIEXPORT void JNICALL Java_edu_asu_sma_client_NativeService_runNativeAsyncTask(JNIEnv* env, jobject thiz)
{
  if (sma::asynctaskqueue.empty())
    return;

  auto it = sma::asynctaskqueue.begin();

  it->second();
  sma::asynctaskqueue.erase(it);
}