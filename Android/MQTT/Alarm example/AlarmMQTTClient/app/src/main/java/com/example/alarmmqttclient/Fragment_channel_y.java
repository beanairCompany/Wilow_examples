package com.example.alarmmqttclient;

import android.icu.util.Calendar;
import android.os.Bundle;

import androidx.fragment.app.Fragment;

import android.os.Handler;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.github.mikephil.charting.charts.LineChart;
import com.github.mikephil.charting.components.XAxis;
import com.github.mikephil.charting.data.Entry;
import com.github.mikephil.charting.data.LineData;
import com.github.mikephil.charting.data.LineDataSet;
import com.github.mikephil.charting.formatter.ValueFormatter;

import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.MqttCallbackExtended;
import org.eclipse.paho.client.mqttv3.MqttMessage;

import java.text.SimpleDateFormat;

import static com.example.alarmmqttclient.AlarmTools.*;

/**
 * A simple {@link Fragment} subclass.
 * Use the {@link Fragment_channel_y#newInstance} factory method to
 * create an instance of this fragment.
 */
public class Fragment_channel_y extends Fragment {

    TextView tv_alarm_chY;
    LineChart chart;
    long tsLong;

    public Fragment_channel_y() {
        // Required empty public constructor
    }


    public static Fragment_channel_y newInstance() {
        Fragment_channel_y fragment = new Fragment_channel_y();
        return fragment;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setUserVisibleHint(false);
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        return inflater.inflate(R.layout.fragment_channel_y, container, false);
    }

    @Override
    public void setUserVisibleHint(boolean isVisibleToUser) {
        super.setUserVisibleHint(isVisibleToUser);
        if(isVisibleToUser){
            // each time this fragment is visible, clear chart and set the MQTT on message received callback to display data in this chart for this channel
            final Handler handler = new Handler();
            handler.postDelayed(new Runnable() {
                @Override
                public void run() {
                    final String ChanneTopic = MqttClient_alarm.deviceId+"/SENSOR/2";
                    tv_alarm_chY =  (TextView) getView().findViewById(R.id.tv_alarm_chY);
                    tsLong = System.currentTimeMillis();
                    chart = (LineChart)getView().findViewById(R.id.chart_y);
                    chart.setHighlightPerTapEnabled(true);
                    chart.setTouchEnabled(true);
                    chart.setDragEnabled(true);
                    chart.setScaleEnabled(true);
                    chart.setPinchZoom(true);
                    chart.setDrawGridBackground(true);
                    chart.getDescription().setEnabled(false);

                    LineData data = new LineData();
                    chart.setData(data);

                    XAxis xAxis = chart.getXAxis();
                    xAxis.setGranularity(1f);
                    xAxis.setLabelRotationAngle(20f);
                    xAxis.setPosition(XAxis.XAxisPosition.BOTTOM);
                    xAxis.setDrawGridLines(false);
                    xAxis.setValueFormatter(new ValueFormatter(){
                        @Override
                        public String getFormattedValue(float timestampMs) {
                            SimpleDateFormat formatter = new SimpleDateFormat("dd/MM/yyyy HH:mm:ss");
                            Calendar calendar = Calendar.getInstance();
                            calendar.setTimeInMillis((long)timestampMs+tsLong);
                            String dateString = formatter.format(calendar.getTime());
                            return  dateString;
                        }
                    });
                    MqttClient_alarm.subscribe(ChanneTopic, new MqttCallbackExtended()
                    {
                        @Override
                        public void connectComplete(boolean reconnect, String serverURI) {

                        }

                        @Override
                        public void connectionLost(Throwable cause) {

                        }

                        @Override
                        public void messageArrived(String topic, MqttMessage mqttMessage) {
                            if (isAlarm(mqttMessage.getPayload())) {
                                if(topic.equals(ChanneTopic)) {
                                    tv_alarm_chY.setText(get_alarmStatus(mqttMessage.getPayload()));
                                    String legend = get_deviceType(mqttMessage.getPayload()) + ": " + topic.substring(0, topic.indexOf("/SENSOR"));
                                    addEntity(get_timestampMs(mqttMessage.getPayload()), get_measurement(mqttMessage.getPayload()), legend);
                                }
                            }
                        }

                        @Override
                        public void deliveryComplete(IMqttDeliveryToken token) {

                        }

                    });
                }
            }, 2000);
        }
    }

    private void addEntity(long x, float y, String Legend){
        LineData data = chart.getData();
        if(data != null){
            LineDataSet set = (LineDataSet) data.getDataSetByIndex(0);
            if(set == null){
                set = new LineDataSet(null, Legend);
                data.addDataSet(set);
            }
            data.addEntry(new Entry( x-tsLong, y),0);
            if(data.getEntryCount()>10){
                Entry e = set.getEntryForXValue(set.getEntryCount() - 1, Float.NaN);
                data.removeEntry(e, 0);
            }
            data.notifyDataChanged();
            chart.notifyDataSetChanged();
            chart.invalidate();
        }
    }
}
