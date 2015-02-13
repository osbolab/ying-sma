package edu.asu.ying.sma.client;

import android.app.ActionBar;
import android.app.Activity;
import android.app.Fragment;
import android.app.FragmentManager;
import android.app.FragmentTransaction;
import android.os.Bundle;
import android.support.v4.widget.DrawerLayout;
import android.view.Menu;

import edu.asu.ying.sma.client.LocalNodeFragment.LocalNodeFragmentContainer;
import edu.asu.ying.sma.client.NeighborhoodFragment.NeighborhoodFragmentContainer;
import edu.asu.ying.sma.client.UdpDemoFragment.UdpDemoFragmentContainer;


public class MainActivity extends Activity
    implements NavigationDrawerFragment.NavigationDrawerCallbacks,
               LocalNodeFragmentContainer,
               NeighborhoodFragmentContainer,
               UdpDemoFragmentContainer {

  /**
   * Fragment managing the behaviors, interactions and presentation of the navigation drawer.
   */
  private NavigationDrawerFragment mNavigationDrawerFragment;

  /**
   * Used to store the last screen title. For use in {@link #restoreActionBar()}.
   */
  private CharSequence last_frag_title;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_main);

    mNavigationDrawerFragment = (NavigationDrawerFragment)
        getFragmentManager().findFragmentById(R.id.navigation_drawer);
    last_frag_title = getTitle();

    // Set up the drawer.
    mNavigationDrawerFragment.setUp(
        R.id.navigation_drawer,
        (DrawerLayout) findViewById(R.id.drawer_layout));

    if (savedInstanceState == null)
      onNavigationDrawerItemSelected(0);
  }

  @Override
  public void onNavigationDrawerItemSelected(int position) {
    // update the main content by replacing fragments
    FragmentManager fragmentManager = getFragmentManager();
    FragmentTransaction txn = getFragmentManager().beginTransaction();
    Fragment frag;
    switch (position) {
      case 0:
        frag = new LocalNodeFragment();
        break;
      case 1:
        frag = new NeighborhoodFragment();
        break;
      case 2:
        frag = new UdpDemoFragment();
        break;
      default:
        throw new IllegalArgumentException("No fragment for navbar item " + position);
    }
    getFragmentManager().beginTransaction()
                        .replace(R.id.container, frag)
                        .commit();
  }

  public void onSectionAttached(int number) {
    switch (number) {
      case 1:
        last_frag_title = getString(R.string.title_localnode);
        break;
      case 2:
        last_frag_title = getString(R.string.title_neighborhood);
        break;
      case 3:
        last_frag_title = getString(R.string.title_udp_demo);
        break;
    }
  }

  public void restoreActionBar() {
    ActionBar actionBar = getActionBar();
    if (actionBar != null) {
      actionBar.setNavigationMode(ActionBar.NAVIGATION_MODE_STANDARD);
      actionBar.setDisplayShowTitleEnabled(true);
      actionBar.setTitle(last_frag_title);
    }
  }


  @Override
  public boolean onCreateOptionsMenu(Menu menu) {
    if (!mNavigationDrawerFragment.isDrawerOpen()) {
      // Only show items in the action bar relevant to this screen
      // if the drawer is not showing. Otherwise, let the drawer
      // decide what to show in the action bar.
      restoreActionBar();
      return true;
    }
    return super.onCreateOptionsMenu(menu);
  }
}
