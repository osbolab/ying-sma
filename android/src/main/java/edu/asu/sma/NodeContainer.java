package edu.asu.sma;

/**
 * @author matt@osbolab.com (Matt Barnard)
 */
public class NodeContainer {
  static {
    System.loadLibrary("smanative");
  }

  public native void create();

  public native void dispose();
}
