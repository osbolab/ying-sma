package edu.asu.sma;

import android.app.Activity;
import android.os.Bundle;
import android.widget.TextView;


public class HelloJniActivity extends Activity {
  /**
   * Called when the activity is first created.
   */
  @Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);

        /* Create a TextView and set its content.
         * the text is retrieved by calling a native
         * function.
         */
    TextView tv = new TextView(this);
    tv.setText(new NodeContainer().stringFromJNI());
    setContentView(tv);
  }
}
