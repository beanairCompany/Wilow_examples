package com.example.lowdutycyclemqttclient;

import android.icu.util.Calendar;
import android.os.Bundle;
import android.os.Handler;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;

import com.github.mikephil.charting.charts.LineChart;
import com.github.mikephil.charting.components.XAxis;
import com.github.mikephil.charting.data.Entry;
import com.github.mikephil.charting.data.LineData;
import com.github.mikephil.charting.data.LineDataSet;
import com.github.mikephil.charting.formatter.ValueFormatter;

import org.eclipse.paho.client.mqttv3.IMqttActionListener;
import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.IMqttToken;
import org.eclipse.paho.client.mqttv3.MqttCallbackExtended;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;

import java.io.UnsupportedEncodingException;
import java.text.SimpleDateFormat;

import static com.example.lowdutycyclemqttclient.LdcTools.get_deviceType;
import static com.example.lowdutycyclemqttclient.LdcTools.get_measurement;
import static com.example.lowdutycyclemqttclient.LdcTools.get_timestampMs;
import static com.example.lowdutycyclemqttclient.LdcTools.isLDC;

public class Fragment_channel_x extends Fragment {

    LineChart chart;
    long tsLong;

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater,@Nullable ViewGroup container,@Nullable Bundle savedInstanceState) {

        return inflater.inflate(R.layout.fragment_channel_x, container, false);
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setUserVisibleHint(false);
    }


    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);
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
                    final String ChanneTopic = MqttClient_ldc.deviceId+"/SENSOR/1";
                    tsLong = System.currentTimeMillis();
                    chart = (LineChart)getView().findViewById(R.id.chart_x);
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
                    MqttClient_ldc.subscribe(ChanneTopic, new MqttCallbackExtended()
                    {
                        @Override
                        public void connectComplete(boolean reconnect, String serverURI) {

                        }

                        @Override
                        public void connectionLost(Throwable cause) {

                        }

                        @Override
                        public void messageArrived(String topic, MqttMessage mqttMessage) throws UnsupportedEncodingException {
                            if(topic.equals(ChanneTopic)) {
                                if (isLDC(mqttMessage.getPayload())) {
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
