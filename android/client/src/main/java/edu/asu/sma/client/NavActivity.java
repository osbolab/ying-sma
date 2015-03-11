package edu.asu.sma.client;

import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentTransaction;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.ImageView;
import android.widget.TabHost;
import android.widget.TabHost.OnTabChangeListener;


public class NavActivity extends FragmentActivity
    implements OnTabChangeListener {

  private TabHost tab_host;
  private int[] tab_icon_res;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_nav);

    tab_icon_res = new int[]{
        R.drawable.tab_ic_network
        , R.drawable.tab_ic_content
        , R.drawable.tab_ic_post
    };

    tab_host = (TabHost) findViewById(android.R.id.tabhost);
    tab_host.setOnTabChangedListener(this);
    tab_host.setup();

    for (int i = 0; i < tab_icon_res.length; ++i)
      createTab(i);

    startService(new Intent(this, TimeService.class));
    startService(new Intent(this, NativeService.class));
  }

  @Override
  protected void onStart() {
    super.onStart();
  }

  @Override
  protected void onStop() {
    super.onStop();
  }

  private void shutdown() {
    stopService(new Intent(this, TimeService.class));
    stopService(new Intent(this, NativeService.class));
    finish();
  }

  private View createTabView(final int index) {
    View view = LayoutInflater.from(this).inflate(R.layout.tab_content, null);
    ImageView image_view = (ImageView) view.findViewById(R.id.tab_icon);
    image_view.setImageDrawable(getResources().getDrawable(tab_icon_res[index]));
    return view;
  }

  private void createTab(final int index) {
    TabHost.TabSpec spec = tab_host.newTabSpec(Integer.toString(index));
    spec.setContent(new TabHost.TabContentFactory() {
      public View createTabContent(String tag) {
        return findViewById(R.id.realtabcontent);
      }
    });
    spec.setIndicator(createTabView(index));
    tab_host.addTab(spec);
  }

  @NonNull
  private Fragment createTabFragment(int index) {
    switch (index) {
      case 0:
        return new NetworkFragment();
      case 1:
        return new ContentFragment();
      case 2:
        return new PostFragment();
    }
    throw new IndexOutOfBoundsException(index + " is not a valid tab index");
  }

  @Override
  public boolean onCreateOptionsMenu(Menu menu) {
    getMenuInflater().inflate(R.menu.nav, menu);
    return super.onCreateOptionsMenu(menu);
  }

  @Override
  public void onTabChanged(String tab_name) {
    final int tab_index = Integer.parseInt(tab_name);

    FragmentTransaction txn = getSupportFragmentManager().beginTransaction();
    txn.replace(R.id.realtabcontent, createTabFragment(tab_index));
    txn.commit();
  }

  @Override
  public boolean onOptionsItemSelected(MenuItem item) {
    switch (item.getItemId()) {
      case R.id.action_upload_logs:
        StatLogger.upload("192.168.1.1", 9996);
        return true;
      case R.id.action_shutdown:
        shutdown();
        return true;
      default:
        return super.onOptionsItemSelected(item);
    }
  }
}
