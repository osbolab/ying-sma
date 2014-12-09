package edu.asu.ying.sma.client;

import android.app.Activity;
import android.app.Fragment;
import android.content.Context;
import android.graphics.Color;
import android.net.DhcpInfo;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;

import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

public class UdpDemoFragment extends Fragment {
  public interface UdpDemoFragmentContainer {
  }

  @Override
  public void onActivityCreated(Bundle savedInstanceState) {
    super.onActivityCreated(savedInstanceState);

    Activity activity = getActivity();

    state_text = (TextView) activity.findViewById(R.id.textState);
    send_indicator = (TextView) activity.findViewById(R.id.send_text);

    received_adapter = new ArrayAdapter<>(getActivity(),
                                          android.R.layout.simple_list_item_1,
                                          received_items);
    ((ListView) activity.findViewById(R.id.listReceived)).setAdapter(received_adapter);

    WifiManager wifi = (WifiManager) activity.getSystemService(Context.WIFI_SERVICE);
    try {
      byte[] send_data = getMacAddress(wifi).getBytes("UTF-8");
      byte[] packet_data = new byte[send_data.length + 1];
      packet_data[0] = (byte) send_data.length;
      System.arraycopy(send_data, 0, packet_data, 1, send_data.length);
      send_packet = new DatagramPacket(packet_data,
                                       packet_data.length,
                                       getBroadcastAddress(wifi),
                                       9999);
    } catch (UnsupportedEncodingException ex) {
      Log.e("Udp Demo", "UTF-8 not supported.", ex);
    } catch (IOException ex) {
      Log.e("Udp Demo", "Exception getting broadcast address", ex);
    }
  }

  @Override
  public View onCreateView(LayoutInflater inflater, ViewGroup container,
                           Bundle savedInstanceState) {
    return inflater.inflate(R.layout.fragment_udp_demo, container, false);
  }

  @Override
  public void onAttach(Activity activity) {
    super.onAttach(activity);
    try {
      container = (UdpDemoFragmentContainer) activity;
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

  @Override
  public void onResume() {
    super.onResume();

    try {
      sock = new DatagramSocket(9999);
    } catch (SocketException e) {
      Log.e("Udp Demo", "Can't listen on port 9999");
      return;
    }

    listen_thread = new ListenThread();
    listen_thread.start();

    send_scheduler = Executors.newScheduledThreadPool(1);
    send_scheduler.scheduleAtFixedRate(new SendTask(), 1, 1, TimeUnit.SECONDS);
  }

  @Override
  public void onPause() {
    super.onPause();

    if (send_scheduler != null) {
      send_scheduler.shutdownNow();
      send_scheduler = null;
    }

    if (sock != null) {
      sock.close();
      sock = null;
    }

    if (listen_thread != null) {
      try {
        listen_thread.join();
      } catch (InterruptedException ignored) {
      }
      listen_thread = null;
    }
  }

  private void receive(final String item) {
    getActivity().runOnUiThread(new Runnable() {
      @Override
      public void run() {
        if (received_items.size() >= 9)
          received_items.clear();
        received_items.add(item);
        received_adapter.notifyDataSetChanged();
      }
    });

  }

  private void indicate_send(final boolean enable) {
    final int color = enable ? Color.argb(0xff, 0, 0, 0) : Color.argb(0xff, 0xcb, 0xcb, 0xcb);
    getActivity().runOnUiThread(new Runnable() {
      @Override
      public void run() {
        send_indicator.setTextColor(color);
      }
    });
  }

  private void set_state(final String state) {
    getActivity().runOnUiThread(new Runnable() {
      @Override
      public void run() {
        state_text.setText(state);
      }
    });
  }

  private String getMacAddress(WifiManager wifi) {
    WifiInfo info = wifi.getConnectionInfo();
    return info.getMacAddress();
  }

  private InetAddress getBroadcastAddress(WifiManager wifi) throws IOException {
    DhcpInfo dhcp = wifi.getDhcpInfo();

    int bcast = (dhcp.ipAddress & dhcp.netmask) | ~dhcp.netmask;
    byte[] octets = new byte[4];
    for (int k = 0; k < 4; ++k)
      octets[k] = (byte) ((bcast >> k * 8) & 0xFF);
    return InetAddress.getByAddress(octets);
  }

  private UdpDemoFragmentContainer container;
  private TextView state_text;
  private TextView send_indicator;

  private DatagramSocket sock = null;

  private Thread listen_thread = null;
  private List<String> received_items = new ArrayList<>();
  private ArrayAdapter<String> received_adapter;

  private DatagramPacket send_packet;
  private ScheduledExecutorService send_scheduler = null;

  private class ListenThread extends Thread {
    @Override
    public void run() {
      set_state("listening");

      byte[] buf = new byte[1024];
      final DatagramPacket packet = new DatagramPacket(buf, buf.length);
      while (sock != null) {
        try {
          sock.receive(packet);
        } catch (IOException ex) {
          if (!ex.getMessage().endsWith("closed"))
            Log.e("UdpActivity", "Exception receiving datagram", ex);
          break;
        }

        if (packet.getLength() != 0) {
          byte[] data = packet.getData();
          if (is_loopback(data))
            continue;

          int strlen = data[0];
          try {
            receive(new String(data, 1, strlen, "UTF-8"));
          } catch (UnsupportedEncodingException e) {
            Log.wtf("UdpActivity", "UTF-8 is not supported", e);
            break;
          }
        }
      }

      set_state("idle");
    }

    private boolean is_loopback(byte[] data) {
      byte[] send_data = send_packet.getData();
      int i = 1;
      for (; i < send_data.length; ++i) {
        if (data[i] != send_data[i])
          break;
      }
      return i == send_data.length;
    }
  }

  private class SendTask implements Runnable {
    @Override
    public void run() {
      if (sock == null)
        return;

      indicate_send(true);
      try {
        sock.send(send_packet);
      } catch (IOException ex) {
        Log.e("Udp Demo", "Exception sending datagram", ex);
      }
      try {
        Thread.sleep(150, 0);
      } catch (InterruptedException ignored) {
      }
      indicate_send(false);
    }
  }
}
