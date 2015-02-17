package edu.asu.sma;

/**
 * @author matt@osbolab.com (Matt Barnard)
 */
public class NodeContainer {
  static {
    System.loadLibrary("smajni");
  }

  public native void create();

  public native void dispose();

  public native String stringFromJNI();
}
