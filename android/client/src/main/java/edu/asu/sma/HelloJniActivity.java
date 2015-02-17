package edu.asu.sma;

import android.app.Activity;
import android.os.Bundle;

public class HelloJniActivity extends Activity {
  /**
   * Called when the activity is first created.
   */
  @Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);

    container = new NodeContainer();
    container.create();
  }

  @Override
  protected void onStop() {
    super.onStop();

    container.dispose();
  }

  private NodeContainer container;
}
