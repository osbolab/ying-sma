#include <sma/android/jniasynchelper.hpp>

#include <sma/io/log.hpp>


namespace sma
{
jobject android_service = nullptr;
JNIEnv* android_service_env = nullptr;
std::multimap<std::chrono::nanoseconds, std::function<void()>> asynctaskqueue;

static Logger log("NativeService");
}


JNIEXPORT void JNICALL Java_edu_asu_sma_client_NativeService_captureServicePointer(JNIEnv* env, jobject thiz)
{
  sma::android_service = thiz;
  sma::android_service_env = env;

  sma::log.d("Captured NativeService*");
}

JNIEXPORT void JNICALL Java_edu_asu_sma_client_NativeService_runNativeAsyncTask(JNIEnv* env, jobject thiz)
{
  if (sma::asynctaskqueue.empty())
    return;

  auto it = sma::asynctaskqueue.begin();

  sma::log.d("Popped native async task... running");

  it->second();
  sma::asynctaskqueue.erase(it);
}