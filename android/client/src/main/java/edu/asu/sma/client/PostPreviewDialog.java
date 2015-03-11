package edu.asu.sma.client;

import android.app.Dialog;
import android.graphics.Color;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.os.Handler;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v4.app.DialogFragment;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.view.Window;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.ImageView.ScaleType;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;

import edu.asu.sma.ContentHelper;

/**
 * @author matt@osbolab.com (Matt Barnard)
 */
public class PostPreviewDialog extends DialogFragment {
  public static PostPreviewDialog newInstance(String assetPath) {
    PostPreviewDialog d = new PostPreviewDialog();

    Bundle args = new Bundle();
    args.putString("asset", assetPath);
    d.setArguments(args);

    return d;
  }

  private final OnClickListener shareListener = new OnClickListener() {
    @Override
    public void onClick(View view) {
      shareData = new ByteArrayOutputStream();

      try {
        byte[] buf = new byte[8192];
        InputStream in = getActivity().getAssets().open(path);
        int read;
        while ((read = in.read(buf)) != -1)
          shareData.write(buf, 0, read);
      } catch (IOException e) {
        throw new RuntimeException(e);
      }

      nativeHandler.post(new Runnable() {
        @Override
        public void run() {
          ContentHelper.create(new String[]{ "dogs" },
                               "dog.jpg",
                               shareData.toByteArray(),
                               shareData.size());
        }
      });

      dismiss();
    }
  };

  private static final String TAG = PostPreviewDialog.class.getSimpleName();

  private Handler nativeHandler;
  private String path;
  private ByteArrayOutputStream shareData = null;

  public void setNativeHandler(Handler handler) {
    nativeHandler = handler;
  }

  @Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    path = getArguments().getString("asset");
  }

  @NonNull
  @Override
  public Dialog onCreateDialog(Bundle savedInstanceState) {
    Dialog dialog = super.onCreateDialog(savedInstanceState);
    dialog.getWindow().requestFeature(Window.FEATURE_NO_TITLE);
    return dialog;
  }

  @Nullable
  @Override
  public View onCreateView(LayoutInflater inflater,
                           ViewGroup container,
                           Bundle savedInstanceState) {
    View view = inflater.inflate(R.layout.post_preview_dialog, container, false);

    view.findViewById(R.id.cancelButton).setOnClickListener(new OnClickListener() {
      @Override
      public void onClick(View view) {
        dismiss();
      }
    });

    Button shareButton = (Button) view.findViewById(R.id.shareButton);

    ImageView imageView = (ImageView) view.findViewById(R.id.imageView);
    imageView.setScaleType(ScaleType.FIT_CENTER);

    try {
      InputStream in = getActivity().getAssets().open(path);
      Drawable d = Drawable.createFromStream(in, null);
      imageView.setImageDrawable(d);
      shareButton.setOnClickListener(shareListener);

    } catch (IOException e) {
      Log.e(TAG, "Exception previewing file from assets", e);
      imageView.setImageResource(R.drawable.qmark);
      imageView.setScaleX(0.45f);
      imageView.setScaleY(0.45f);
      shareButton.setEnabled(false);
      shareButton.setBackgroundColor(Color.LTGRAY);
    }

    return view;
  }
}
