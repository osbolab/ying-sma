package edu.asu.sma.client;


import android.os.Bundle;
import android.os.Handler;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.ListView;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

import edu.asu.sma.InterestHelper;

public class ContentFragment extends BaseFragment implements NativeConsumer {
  private EditText newInterestText;

  private List<String> interests = new ArrayList<>();
  private ArrayAdapter<String> interestsAdapter;

  private ScheduledExecutorService executor = Executors.newSingleThreadScheduledExecutor();


  public ContentFragment() {
    bind(this);
  }

  @Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
  }

  @Override
  public View onCreateView(LayoutInflater inflater, ViewGroup container,
                           Bundle savedInstanceState) {
    return inflater.inflate(R.layout.fragment_content, container, false);
  }

  @Override
  public void onActivityCreated(Bundle savedInstanceState) {
    super.onActivityCreated(savedInstanceState);

    newInterestText = (EditText) activity.findViewById(R.id.newInterestText);

    interestsAdapter = new ArrayAdapter<>(activity,
                                          android.R.layout.simple_list_item_1,
                                          interests);
    ListView interestsList = (ListView) activity.findViewById(R.id.interestsList);
    interestsList.setAdapter(interestsAdapter);

    activity.findViewById(R.id.addInterestButton).setOnClickListener(new OnClickListener() {
      @Override
      public void onClick(View view) {
        String interest = newInterestText.getText().toString();
        if (interest.length() == 0)
          return;

        InterestHelper.create(interest);

        newInterestText.setText("");
      }
    });

    update();
  }

  private void update() {
    if (nativeHandler == null)
      return;

    nativeHandler.post(new Runnable() {
      @Override
      public void run() {
        final List<String> local_interests = InterestHelper.local();
        final List<String> remote_interests = InterestHelper.remote();
        java.util.Collections.sort(local_interests);
        java.util.Collections.sort(remote_interests);

        activity.runOnUiThread(new Runnable() {
          @Override
          public void run() {
            interests.clear();
            interestsAdapter.addAll(local_interests);
            for (String remote : remote_interests)
              interestsAdapter.add(remote + " (remote)");
          }
        });
      }
    });

    executor.schedule(new Runnable() {
      @Override
      public void run() {
        update();
      }
    }, 500, TimeUnit.MILLISECONDS);
  }

  @Override
  public void onNativeHandler(Handler handler) {
    update();
  }
}
