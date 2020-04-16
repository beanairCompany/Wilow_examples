package com.example.lowdutycyclemqttclient;


import android.content.Context;
import android.util.Log;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;

import org.eclipse.paho.android.service.MqttAndroidClient;
import org.eclipse.paho.client.mqttv3.IMqttActionListener;
import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.IMqttToken;
import org.eclipse.paho.client.mqttv3.MqttCallbackExtended;
import org.eclipse.paho.client.mqttv3.MqttConnectOptions;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;

public class MqttClient_ldc extends AppCompatActivity{

    public static String deviceId = "";
    private static MqttClient_ldc ldcMqttClient = null;
    private static String topic = "";
    private static boolean isSub_chZ = false;
    private static boolean isSub_chX = false;
    private static boolean isSub_chY = false;
    private static boolean isSub_IncX = false;
    private static boolean isSub_IncY = false;

    public static MqttAndroidClient mqttAndroidClient;
    private  MqttClient_ldc(Context context, String brokerIp, String brokerPort, String clientId, IMqttActionListener iMqttActionListener){
        try {
            if(ldcMqttClient == null) {
                //MqttAndroidClient mqttAndroidClient = new MqttAndroidClient(getApplicationContext(), t_brokerId.getText().toString(), t_clientId.getText().toString());
                mqttAndroidClient = new MqttAndroidClient(context, "tcp://" + brokerIp+":"+brokerPort, clientId);
                MqttConnectOptions mqttConnectOptions = new MqttConnectOptions();
                mqttAndroidClient.connect(mqttConnectOptions, null, iMqttActionListener);
            }
        } catch (MqttException e) {
            e.printStackTrace();
        }
    }
/*
    public static MqttClient_ldc connectToBroker(Context context, String brokerIp, String brokerPort, String clientId, IMqttActionListener iMqttActionListener){
        if(ldcMqttClient==null)
            ldcMqttClient = new MqttClient_ldc(context, brokerIp, brokerPort, clientId, iMqttActionListener);
        return ldcMqttClient;
    }
*/
    public static MqttClient_ldc connectToBroker(Context context, String brokerIp, String brokerPort, String clientId, IMqttActionListener iMqttActionListener){
        mqttAndroidClient = new MqttAndroidClient(context, "tcp://" + brokerIp+":"+brokerPort, clientId);
        MqttConnectOptions mqttConnectOptions = new MqttConnectOptions();
        try {
            mqttAndroidClient.connect(mqttConnectOptions, null, iMqttActionListener);
        } catch (MqttException e) {
            e.printStackTrace();
        }
        return ldcMqttClient;
    }

    public static void subscribe(final String newTopic, final MqttCallbackExtended mqttCallbackExtended){
        try {
            if(!topic.equals("")) {
                mqttAndroidClient.unsubscribe(topic);
            }
            mqttAndroidClient.subscribe(newTopic, 0, null, new IMqttActionListener() {
                @Override
                public void onSuccess(IMqttToken asyncActionToken) {
                    topic = newTopic;
                    mqttAndroidClient.setCallback(mqttCallbackExtended);
                }

                @Override
                public void onFailure(IMqttToken asyncActionToken, Throwable exception) {

                }
            });
        } catch (MqttException e) {
            e.printStackTrace();
        }
    }

    public void subToChZ(){
        try {
            if(isSub_chZ==false){
                isSub_chZ=true;
                mqttAndroidClient.subscribe("F0B5D1A48F4E0000/SENSOR/0", 0, null, new IMqttActionListener() {
                    @Override
                    public void onSuccess(IMqttToken asyncActionToken) {
                        mqttAndroidClient.setCallback(new MqttCallbackExtended()
                        {
                            @Override
                            public void connectComplete(boolean reconnect, String serverURI) {

                            }

                            @Override
                            public void connectionLost(Throwable cause) {

                            }

                            @Override
                            public void messageArrived(String topic, MqttMessage mqttMessage) {

                            }

                            @Override
                            public void deliveryComplete(IMqttDeliveryToken token) {

                            }

                        });
                    }

                    @Override
                    public void onFailure(IMqttToken asyncActionToken, Throwable exception) {

                    }
                });


            }
        } catch (MqttException e) {
            e.printStackTrace();
        }
    }

}
