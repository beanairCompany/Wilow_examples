package com.example.alarmmqttclient;

import android.icu.util.Calendar;

import java.text.SimpleDateFormat;

public class AlarmTools {
    // get device type, return string
    static String get_deviceType(byte[]payload){
        if(payload[0]==0x01)
            return "AX 3D";
        else if(payload[0]==0x02)
            return "HI INC MONO" ;
        else if(payload[0]==0x03)
            return "HI INC BI";
        else if(payload[0]==0x04)
            return "X- INC MONO";
        else if(payload[0]==0x05)
            return "X- INC BI";
        else if(payload[0]==0x06)
            return "AX 3DS";
        return "unknown";
    }
    // get dac mode, return string
    static String get_dacMode(byte[]payload){
        if(payload[1]==0x01)
            return "LowDutyCycle";
        else if(payload[1]==0x02)
            return "Alarm";
        else if(payload[1]==0x03)
            return "Streaming";
        else if(payload[1]==0x04)
            return "Shock Detection";
        else if(payload[1]==0x05)
            return "Ldc Math Result";
        else if(payload[1]==0x06)
            return "S.E.T";
        else if(payload[1]==0x07)
            return"Dynamic math result";
        return "unknown";
    }
    // get channel name, return string
    static String get_chanelName(byte[]payload){
        if(payload[2]==0x01)
            return "Ch_Z";
        else if(payload[2]==0x02)
            return "Ch_X";
        else if(payload[2]==0x03)
            return "Ch_Y";
        else if(payload[2]==0x04)
            return "Inc_X";
        else if(payload[2]==0x05)
            return "Inc_Y";
        return "unknown";
    }
    // get channel name, return string
    static String get_alarmStatus(byte[]payload){
        if(payload[7]==0x00)
            return "No Alarm";
        else if(payload[7]==0x01)
            return "Alarm start";
        else if(payload[7]==0x02)
            return "Alarm in progress";
        else if(payload[7]==0x03)
            return "Alarm end";
        return "unknown";
    }
    // get timestamp, return long
    /*static double get_timestampMs(byte[]payload){
        double timestamp = 0x0000000000000000;
        timestamp += payload[3]&0xff;
        timestamp += (payload[4]&0xff)<<8;
        timestamp += (payload[5]&0xff)<<16;
        timestamp += (payload[6]&0xff)<<24;
        return  timestamp*1000;
    }*/
    // get timestamp, return long
    static long get_timestampMs(byte[]payload){
        long timestamp = 0x0000000000000000;
        timestamp |= payload[3]&0xff;
        timestamp |= (payload[4]&0xff)<<8;
        timestamp |= (payload[5]&0xff)<<16;
        timestamp |= (payload[6]&0xff)<<24;
        return  timestamp*1000;
    }
    // get time, return string
    static String get_dateTime(byte[]payload){
        double timestamp =  get_timestampMs(payload);
        SimpleDateFormat formatter = new SimpleDateFormat("dd/MM/yyyy HH:mm:ss");
        Calendar calendar = Calendar.getInstance();
        calendar.setTimeInMillis((long) timestamp);
        String dateString = formatter.format(calendar.getTime());
        return  dateString;
    }
    // get data, return float
    static float get_measurement(byte[]payload){
        long value = 0x00000000;
        value += payload[8]&0xff;
        value += (payload[9]&0xff)<<8;
        value += (payload[10]&0x7f)<<16;
        if((payload[10]&0x80)==0x80){
            value *= -1;
        }
        return ((float)value/1000);
    }

    static boolean isAlarm(byte[]payload){
        if(get_dacMode(payload).equals("Alarm")){
            return true;
        }
        return false;
    }
}
