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

import edu.asu.sma.Node;
import edu.asu.sma.NodeContainer;

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


  public NativeService() {
  }

  @Override
  public synchronized void onCreate() {
    super.onCreate();
    HandlerThread thread = new HandlerThread("NativeService_Background",
                                             Process.THREAD_PRIORITY_BACKGROUND);
    thread.start();
    looper = thread.getLooper();
    handler = new Handler(looper);

    if (NodeContainer.create()) {
      Log.d(TAG, "Created native node on background thread");
      tick();
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
  }

  @Override
  public IBinder onBind(Intent intent) {
    return new BinderWrapper();
  }

  private void tick() {
    if (Thread.currentThread() == looper.getThread()) {
      Node.tick();
      try {
        Thread.sleep(TICK_PERIOD_MS, 0);
      } catch (InterruptedException ignored) {
      }
    }

    if (looper != null)
      handler.post(new Runnable() {
        @Override
        public void run() {
          tick();
        }
      });
  }
}
