package com.example.lowdutycyclemqttclient;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

import com.example.lowdutycyclemqttclient.MqttClient_ldc;

import org.eclipse.paho.client.mqttv3.IMqttActionListener;
import org.eclipse.paho.client.mqttv3.IMqttToken;

public class MainActivity extends AppCompatActivity {

    Button b_connect;
    EditText t_clientId;
    EditText t_brokerIp ;
    EditText t_brokerPort;
    EditText t_deviceId;
    TextView tv_connectionStatus;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        b_connect = (Button) findViewById(R.id.b_connect);
        t_clientId = (EditText) findViewById(R.id.t_clientId);
        t_brokerIp = (EditText) findViewById(R.id.t_brokerIp);
        t_brokerPort = (EditText) findViewById(R.id.t_brokerPort);
        t_deviceId = (EditText) findViewById(R.id.t_deviceId);
        tv_connectionStatus = (TextView) findViewById(R.id.tv_connectionStatus);

        b_connect.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
            openMonitor();
            }
        });
    }

    public void openMonitor(){
        tv_connectionStatus.setText("Connecting...");
        tv_connectionStatus.setTextColor(Color.BLACK);
        MqttClient_ldc.connectToBroker(getApplicationContext(), t_brokerIp.getText().toString(), t_brokerPort.getText().toString(), t_clientId.getText().toString(), new IMqttActionListener() {
            @Override
            public void onSuccess(IMqttToken asyncActionToken) {
                tv_connectionStatus.setText("Connection done!");
                MqttClient_ldc.deviceId = t_deviceId.getText().toString();
                Intent intent = new Intent(getApplicationContext(), Monitor.class);
                startActivity(intent);
            }

            @Override
            public void onFailure(IMqttToken asyncActionToken, Throwable exception) {
                tv_connectionStatus.setText("Connection failed!");
                tv_connectionStatus.setTextColor(Color.RED);
            }
        });

    }
}
