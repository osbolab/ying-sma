package edu.asu.sma.client;


import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import edu.asu.sma.ContentHelper;

public class PostFragment extends BaseFragment {
  private List<String> items = new ArrayList<>();
  private ArrayAdapter<String> itemsAdapter;

  public PostFragment() {
  }

  @Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
  }

  @Override
  public View onCreateView(LayoutInflater inflater, ViewGroup container,
                           Bundle savedInstanceState) {
    return inflater.inflate(R.layout.fragment_post, container, false);
  }

  @Override
  public void onActivityCreated(Bundle savedInstanceState) {
    super.onActivityCreated(savedInstanceState);

    itemsAdapter = new ArrayAdapter<>(activity, android.R.layout.simple_list_item_1, items);
    final ListView itemsList = (ListView) activity.findViewById(R.id.postItemsList);
    itemsList.setAdapter(itemsAdapter);

    itemsList.setOnItemClickListener(new OnItemClickListener() {
      @Override
      public void onItemClick(AdapterView<?> adapterView, View view, int i, long l) {
        final ByteArrayOutputStream fileData = new ByteArrayOutputStream();

        TextView item = (TextView) adapterView.getChildAt(i - itemsList.getFirstVisiblePosition());
        String fileName = item.getText().toString();
        try {
          byte[] buf = new byte[8192];
          InputStream in = activity.getAssets().open(fileName);
          int read;
          while ((read = in.read(buf)) != -1)
            fileData.write(buf, 0, read);
        } catch (IOException e) {
          throw new RuntimeException(e);
        }

        Log.d("Post", "Read " + fileData.size() + " bytes of file data");

        nativeHandler.post(new Runnable() {
          @Override
          public void run() {
            ContentHelper.create(new String[]{ "dogs" },
                                 "dog.jpg",
                                 fileData.toByteArray(),
                                 fileData.size());
          }
        });
      }
    });

    try {
      String[] files = activity.getAssets().list("");
      items.clear();
      Collections.addAll(items, files);
      itemsAdapter.notifyDataSetChanged();
    } catch (IOException e) {
      throw new RuntimeException(e);
    }
  }
}
