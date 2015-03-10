package edu.asu.sma;

/**
 * @author matt@osbolab.com (Matt Barnard)
 */
public final class ContentHelper {
  private ContentHelper() {
  }

  public static native void create(String[] types, String name, byte[] data, long size);

  static {
    System.loadLibrary(Sma.LIBRARY_NAME);
  }
}
