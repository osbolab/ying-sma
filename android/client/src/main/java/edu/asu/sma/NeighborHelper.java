package edu.asu.sma;

import java.util.ArrayList;

/**
 * @author matt@osbolab.com (Matt Barnard)
 */
public final class NeighborHelper {
  public static native ArrayList<String> all();

  static {
    System.loadLibrary(Sma.LIBRARY_NAME);
  }
}
