package edu.asu.sma.client;

import android.os.Handler;

/**
 * @author matt@osbolab.com (Matt Barnard)
 */
public interface NativeConsumer {
  void onNativeHandler(Handler handler);
}
