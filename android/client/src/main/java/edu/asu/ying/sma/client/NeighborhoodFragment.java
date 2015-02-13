package edu.asu.ying.sma.client;

import android.app.Activity;
import android.app.Fragment;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

public class NeighborhoodFragment extends Fragment {
  public interface NeighborhoodFragmentContainer {
  }

  public NeighborhoodFragment() {
  }

  @Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
  }

  @Override
  public View onCreateView(LayoutInflater inflater, ViewGroup container,
                           Bundle savedInstanceState) {
    // Inflate the layout for this fragment
    return inflater.inflate(R.layout.fragment_local_node, container, false);
  }

  @Override
  public void onAttach(Activity activity) {
    super.onAttach(activity);
    try {
      container = (NeighborhoodFragmentContainer) activity;
    } catch (ClassCastException e) {
      throw new ClassCastException(activity.toString()
                                   + " must implement OnFragmentInteractionListener");
    }
  }

  @Override
  public void onDetach() {
    super.onDetach();
    container = null;
  }

  private NeighborhoodFragmentContainer container;
}
