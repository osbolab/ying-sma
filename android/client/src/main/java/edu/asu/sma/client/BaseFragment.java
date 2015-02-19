package edu.asu.sma.client;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.support.v4.app.Fragment;

/**
 * @author matt@osbolab.com (Matt Barnard)
 */
public abstract class BaseFragment extends Fragment {
  protected NavActivity activity;
  private NativeConsumer consumer = null;
  protected Handler nativeHandler = null;

  private ServiceConnection svc_conn = new ServiceConnection() {
    @Override
    public void onServiceConnected(ComponentName componentName, IBinder binder) {
      nativeHandler = ((NativeHandlerBinder) binder).nativeHandler();
      if (consumer != null)
        consumer.onNativeHandler(nativeHandler);
    }

    @Override
    public void onServiceDisconnected(ComponentName componentName) {
      nativeHandler = null;
    }
  };

  protected void bind(NativeConsumer consumer) {
    this.consumer = consumer;
  }

  @Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    activity = (NavActivity) getActivity();
    activity.bindService(new Intent(activity, NativeService.class),
                         svc_conn,
                         Context.BIND_AUTO_CREATE | Context.BIND_NOT_FOREGROUND);
  }

  @Override
  public void onStop() {
    super.onStop();
    if (nativeHandler != null) {
      activity.unbindService(svc_conn);
      nativeHandler = null;
    }
  }
}
