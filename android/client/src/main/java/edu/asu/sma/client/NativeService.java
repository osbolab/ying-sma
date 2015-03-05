package edu.asu.sma.client;

import android.app.Service;
import android.content.Intent;
import android.os.Binder;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Process;
import android.util.Log;

import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

import edu.asu.sma.Node;
import edu.asu.sma.NodeContainer;
import edu.asu.sma.Sma;

public class NativeService extends Service {
  private final class BinderWrapper extends Binder implements NativeHandlerBinder {
    @Override
    public Handler nativeHandler() {
      return handler;
    }
  }

  private static final String TAG = NativeService.class.getSimpleName();
  private static final double TICK_FREQ_HZ = 100.0;
  private static final long TICK_PERIOD_MS = (long) (1000.0 / TICK_FREQ_HZ);

  private Looper looper;
  private Handler handler;

  // Continuously invoke native code in the service thread without blocking the UI thread from
  // doing the same.
  private Runnable repeatingTickTask = new Runnable() {
    @Override
    public void run() {
      if (looper == null)
        return;

      if (Thread.currentThread() == looper.getThread()) {
        Node.tick();
        try {
          Thread.sleep(TICK_PERIOD_MS, 0);
        } catch (InterruptedException ignored) {
        }
      }

      handler.post(this);
    }
  };

  // Manages the delayed execution of native async tasks
  private ScheduledExecutorService asyncExecutor = Executors.newScheduledThreadPool(1);
  // Runs the next native task (if any) on the service thread
  private Runnable nativeTaskRunner = new Runnable() {
    @Override
    public void run() {
      if (Thread.currentThread() == looper.getThread())
        runNativeAsyncTask();
      else
        handler.post(this);
    }
  };


  public NativeService() {
    captureServicePointer();
  }

  @Override
  public synchronized void onCreate() {
    super.onCreate();
    HandlerThread thread = new HandlerThread("NativeService_Background",
                                             Process.THREAD_PRIORITY_BACKGROUND);
    thread.start();
    looper = thread.getLooper();
    handler = new Handler(looper);

    if (NodeContainer.create(0)) {
      Log.d(TAG, "Created native node on background thread");
      repeatingTickTask.run();
    } else {
      Log.d(TAG, "Native node already created");
    }
  }

  @Override
  public int onStartCommand(Intent intent, int flags, int startId) {
    return START_STICKY;
  }

  @Override
  public synchronized void onDestroy() {
    super.onDestroy();

    looper.quit();
    looper = null;

    Log.d(TAG, "Destroying native node");
    NodeContainer.dispose();
    deleteServicePointer();
  }

  @Override
  public IBinder onBind(Intent intent) {
    return new BinderWrapper();
  }

  // Called by native code to schedule an async task on the service thread
  public void scheduleNativeAsync(long delay_nanos) {
    asyncExecutor.schedule(nativeTaskRunner, delay_nanos, TimeUnit.NANOSECONDS);
  }

  private native void captureServicePointer();

  private native void deleteServicePointer();

  private native void runNativeAsyncTask();

  static {
    System.loadLibrary(Sma.LIBRARY_NAME);
  }
}
