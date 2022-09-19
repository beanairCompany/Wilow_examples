/**************************************************************************************************************************
 *
 * Company    : BeanAir
 * Date       : 09/05/2022
 * Author     : Marwene mechri
 *
 * Description:
 *   This Example uses MQTT protocol to send a Data acquisition configuration {OTAC} packet to the BeanDevice.
 *   This OTAC is responsible of configuring the acquisition mode (streaming, SET mode, Alarm, Low duty cycle),
 *   it also has the role of configuring the device in TX, log, TX & Log or Stand alone mode.
 *
 *   Data acquisition configuration (DAQ) OTAC Packet content:
 *     +(0)------+(8)----------+(9)------+(10)------+(11)---------+(13)------+(15)------------+(18)------+(19)----------++
 *     |  Mac_Id | Otac_Length | Otac_Id | Daq_Mode | Daq_Options | Reserved | Dac_Duty_Cycle | TX_Ratio | Daq_Duration || 
 *     | (8bytes)|   (1byte)   | (1byte) | (1byte)  |   (2bytes)  | (2bytes) |    (3bytes)    | (1bytes) |   (3bytes)   ||
 *     +---------+-------------+---------+----------+-------------+----------+----------------+----------+--------------++
 *     +(22)-----------+(25)------+(28)----------------------+(30)---------+
 *     | Sampling_Rate | Reserved | Store_Forward_Data_Aging | Notif.ratio | 
 *     |   (3bytes)    | (3bytes) |        (2bytes)          | (2bytes)    |            
 *     +---------------+----------+--------------------------+-------------+
 * 
 *   NOTE:
 *      + Dac_Duty_Cycle ,Daq_Duration ,Sampling_Rate ,Store_Forward_Data_Aging ,Notif.ratio:
 *        The sequence of the bytes in this fields is the least significant byte first (LSB first).
 *      + Store_Forward_Data_Aging field isn't used any more. When you activate the store and forward option
 *        the device takes a default value for the data aging field.
 *      + Dac_Mode status are 6 :
 *            _______________________________
 *           |     Daq_Mode     | Value(Hex) | 
 *           |------------------|------------|
 *           | *Commissioning   |    0x01    |
 *           | *Low duty cycle  |    0x02    |
 *           | *Streaming       |    0x03    |
 *           | *Alarm           |    0x04    |
 *           | *SET mode        |    0x05    |
 *           | *Shock Detection |    0x06    |
 *           |__________________|____________|       
 *   
 *      + The bits significations of the field Daq_Options as below :        
 *         ______________________________________________________________________________________________________________
 *        | Daq_Options bit |                            Signification                                                  |
 *        |-----------------|-------------------------------------------------------------------------------------------|
 *        |      Bit 0      |  Datalogger bit.        | 1 = datalogger enabled.        | 0 = datalogger disabled        |
 *        |_________________|_________________________|________________________________|________________________________|
 *        |      Bit 1      |  Store and forward bit. | 1 = Store and forward enabled. | 0 = Store and forward disabled |     
 *        |_________________|_________________________|________________________________|________________________________| 
 *        |                 |  Streaming Option filed (MSB first).                                                      |      
 *        |                 |    *0b100 = Streaming Continuous.                                                         |
 *        |     Bit 2-->4   |    *0b010 = Streaming one shot.                                                           |
 *        |                 |    *0b110 = Streaming burst.                                                              |
 *        |_________________|___________________________________________________________________________________________|           
 *        |      Bit 5      |  Transmission TX bit.   | 1 = TX enabled                 | 0 = TX disabled                |
 *        |_________________|_________________________|________________________________|________________________________|  
 *        |      Bit 6      |  Stand Alone bit.       | 1 = Stand Alone enabled        | 0 = Stand Alone disabled       |
 *        |_________________|_________________________|________________________________|________________________________| 
 *        |    Bits 7-->15  |  Reserved                                                                                 |
 *        |_________________|___________________________________________________________________________________________|
 *
 * Reference:
 *   For more details, please follow the link below: 
 *   http://www.wireless-iot.beanair.com/files/TN-RF-004-MQTT-Comnmunication-Protocol.pdf (pages: 29 ,30).
 * 
 * The beandevice used to test this Demo_c_code is "H-Inc" with 2 channels active.
 * We use mosquito broker on a local network with an IP address of "192.168.1.91".
 *   or you can use remote one like "broker.hivemq.com", so make sure to:
 *    + set your configuration right before running the example.
 *    + connect this program and the BeanDevice to the same broker.
 * Preparation:
 * 1- Configure the beandevice to use the MQTT mode.
 * 2- Make sure to enable the topic for static measurement of each channel.
 * 3- Start running the beandevice in any mode of acquisition.
 * Steps:
 * 1- connect to the broker.
 * 2- subscribe to data acquisition otac topic {[MAC_ID]/OTAC}.
 * 3- choose the otac from the exaples below from p_DataAcqConfigOtacEX1--->8 to be send to the device. 
 * 3- send an otac with the  Beanair_Mqtt_client_demo_C code by running it.
 * 4- Check to see if the device's configuration has changed since we sent it the new one via this demo. 
 **************************************************************************************************************************/

/*
####################################
############# Libraries ############
####################################
*/

/****** Standards library of c language ******/
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

/****** Paho MQTT C Client library ***********/ 
#include "MQTTClient.h"

/*
####################################
########### Configuration ##########
####################################
*/

/****** Broker configuration ****************/
#define  PORT_BROKER     "1883"                                    /*Change the PORT_BROKER with the port number of your broker.*/
#define  ADDRESS_BROKER  "192.168.1.91"                            /*Change the ADDRESS_BROKER with the IP OR HOSTNAME of your broker.*/
#define  CLIENTID        "ExampleClientSubBeanair"
#define  TIMEOUT         1000L
#define  QOS             1

/****** Beandevice Configuration *****/
#define DEVICE_MAC_ID    "A4D57843DEDD0000"                        /*Change the DEVICE_MAC_ID with the mac id of your device*/

/****** Global variables *************/
#define  TOPIC             DEVICE_MAC_ID "/OTAC"
#define  ADDRESS           ADDRESS_BROKER ":"  PORT_BROKER
#define  TRUE              1
#define  OTAC_LENGTH       32                                      /*The length of buffer that contains the packet.*/



/****************************************************************
 *Example 1:                                                    *
 *  Data Acq.mode            : Streaming.                       *
 *  Sampling Rate            : 10 Hz.                           *
 *  Data acquisition mode    : TX only.                         *
 *  Streaming Packet Options : Continous Monitoring.            *
 *  Store and Forward        : No.                              *  
 ****************************************************************/
  unsigned char a_DataAcqConfigOtacEX1[OTAC_LENGTH]= {
     0xA4 ,0xD5 ,0x78 ,0x43 ,0xDE ,0xDD ,0x00 ,0x00  // The Mac Id (Msb First)                                                                                               
    ,0x17                                            // The OTAC Length is (23)Decimal.                                                                                                
    ,0x10                                            // The OTAC Id (0x10)Hex.                                                                                                    
    ,0x03                                            // The Daq Mod Streaming.                                                                                              
    ,0x24 ,0x00                                      // The Daq options (00100100)Binary datalogger disabled;Store and forward disabled;Streaming Continuous; TX enabled.         
    ,0x00 ,0x00                                      // Future Use                                                                                                         
    ,0x00 ,0x00 ,0x00                                // Daq duty cycle (Lsb first)                                                                                          
    ,0x00                                            // TX Ratio                                                                                                         
    ,0x00 ,0x00 ,0x00                                // Daq duration   (Lsb first)                                                                                          
    ,0x0A ,0x00 ,0x00                                // Sampling Rate  (Lsb first)  ==>(0x00000A)Hex ==>(10)Decimal.  
    ,0x00 ,0x00 ,0x00                                // Future Use   
    ,0x00 ,0x00                                      // Store and forward Data aging (Lsb first).              
    ,0x00 ,0x00                                      // Math Notif.ratio.                     
    };

/****************************************************************
 *Example 2:                                                    *
 *  Data Acq.mode            : Streaming.                       *
 *  Sampling Rate            : 200 Hz.                          *
 *  Data acquisition mode    : TX & Log.                        *
 *  Streaming Packet Options : Continous Monitoring.            *
 *  Store and Forward        : Yes.                             *  
 ****************************************************************/
unsigned char a_DataAcqConfigOtacEX2[OTAC_LENGTH] ={
    0xA4 ,0xD5 ,0x78 ,0x43 ,0xDE ,0xDD ,0x00 ,0x00  // The Mac Id (Msb First)       
    ,0x17                                           // The OTAC Length is 23.    
    ,0x10                                           // The OTAC Id  0x10.      
    ,0x03                                           // The Daq Mod Streaming.                                              
    ,0x27 ,0x00                                     // The Daq options (0b00100111)Binary datalogger enabled;Store and forward enabled;Streaming Continuous;TX enabled.
    ,0x00 ,0x00                                     // Future Use. 
    ,0x00 ,0x00 ,0x00                               // Daq duty cycle (Lsb first).     
    ,0x00                                           // TX Ratio.     
    ,0x00 ,0x00 ,0x00                               // Daq duration   (Lsb first). 
    ,0xC8 ,0x00 ,0x00                               // Sampling Rate  (Lsb first) (0x0000c8)Hex==>(200)Hz.
    ,0x00 ,0x00 ,0x00                               // Future Use.   
    ,0x00 ,0x00                                     // Store and forward Data aging (Lsb first).  
    ,0x00 ,0x00                                     // Math Notif.ratio.  
    };  

/****************************************************************
 *Example 3:                                                    *
 *  Data Acq.mode                 : LowDutyCycle.               *
 *  Data Acq.cycle                : 20s.                        *
 *  Store and Forward             : NO.                         *
 *  Math Notif.ratio              : 2.                          *
 *  Data acquisition mode options : TX & Log.                   *  
 ****************************************************************/
unsigned char a_DataAcqConfigOtacEX3[OTAC_LENGTH] ={
    0xA4,0xD5,0x78,0x43 ,0xDE,0xDD ,0x00,0x00      // The Mac Id (Msb First)                                                                                              
    ,0x17                                          // The OTAC Length is (0x17)Hex==>(23)Decimal.                                                                                             
    ,0x10                                          // The OTAC Id  (0x10)Hex.                                                                                                  
    ,0x02                                          // The Daq Mod low duty cycle.
    ,0x21 ,0x00                                    // (0b00100001)Binary datalogger enabled,Store and forward disabled,TX enabled,Stand Alone disabled.
    ,0x00 ,0x00                                    // Future Use 
    ,0x14 ,0x00 ,0x00                              // Daq duty cycle  (Lsb first)   (10100s)base2  (20s)base10.             
    ,0x01                                          // TX Ratio  (Automatically).  
    ,0x00 ,0x00 ,0x00                              // Daq duration(Lsb first) 
    ,0x00 ,0x00 ,0x00                              // Sampling Rate(Lsb first)
    ,0x00 ,0x00 ,0x00                              // Future Use
    ,0x00 ,0x00                                    // Store and forward Data aging(Lsb first)
    ,0x02 ,0x00                                    // Math Notif.ratio (Lsb first)  (2seconds)Decimal.
    };

/*****************************************************************
 *Example 4:                                                     *
 *  Data Acq.mode                 : Alarm.                       *
 *  Data Acq.cycle                : 80s.                         *     
 *  Tx Ratio                      : 14.                          *  
 *  Data acquisition mode options : Log only.                    *
 *  Store and Forward             : NO.                          *
 *  Math Notif.ratio              : 2.                           *
 *  Store and Forward             : Yes.                         *
 *  |-> Data Aging                : 9.                           *
 ****************************************************************/
unsigned char a_DataAcqConfigOtacEX4[OTAC_LENGTH] ={
    0xA4 ,0xD5 ,0x78 ,0x43 ,0xDE ,0xDD ,0x00 ,0x00 // The Mac Id (Msb First)                                                                                              
    ,0x17                                          // The OTAC Length is (23)Decimal.                                                                                             
    ,0x10                                          // The OTAC Id  0x10.                                                                                                  
    ,0x04                                          // The Daq Mod :Alarm.
    ,0x03 ,0x00                                    // 00000011 datalogger enabled, Store and forward enabled ,TX disabled,Stand Alone disabled.
    ,0x00 ,0x00                                    // Future Use 
    ,0x50 ,0x00 ,0x00                              // Daq duty cycle   (Lsb first)   (01010000)base2  (80)base10 s.             
    ,0x0E                                          // TX Ratio (14)decimal.  
    ,0x00 ,0x00 ,0x00                              // Daq duration(Lsb first). 
    ,0x00 ,0x00 ,0x00                              // Sampling Rate(Lsb first).
    ,0x00 ,0x00 ,0x00                              // Future Use  
    ,0x09 ,0x00                                    // Store and forward Data aging(Lsb first) (9)base10. 
    ,0x00 ,0x00                                    // Math Notif.ratio (Lsb first).
    };

/****************************************************************
 *Example 5:                                                    *
 *  Data Acq.mode            : Streaming.                       *
 *  Sampling Rate            : 800 Hz.                          *
 *  Data acquisition mode    : TX & Log.                        *
 *  Streaming Packet Options : Burst.                           *
 *  Data Acq.cycle           : 12 min.                          *
 *  Data Acq.duration        :  3 min.                          *
 *  Store and Forward        : Yes.                             *
 *  |-->Data Aging           : 437.                             *   
 *  Stand Alone              : No.                              *  
 ****************************************************************/
unsigned char a_DataAcqConfigOtacEX5[OTAC_LENGTH] = {
     0xA4 ,0xD5 ,0x78 ,0x43 ,0xDE ,0xDD ,0x00 ,0x00 // The Mac Id (Msb First)   
    ,0x17                                           // The OTAC Length is (23)Decimal default. 
    ,0x10                                           // The OTAC Id  (0x10)Hex Default.
    ,0x03                                           // The Daq Mod Streaming.   
    ,0x2F ,0x00                                     // (0b00101111)binary datalogger enabled, Store and forward enabled ,
                                                    // (0b110)binary Streming burst ,TX enabled,Stand Alone disabled.
    ,0x00 ,0x00                                     // Future Use 
    ,0xD0 ,0x02 ,0x00                               // Daq duty cycle(Lsb first) Little Endian (720)second in Decimal. 
    ,0x00                                           // TX Ratio.  
    ,0xB4 ,0x00 ,0x00                               // Daq duration(Lsb first) Little Endian (180)second in Decimal.
    ,0x20 ,0x03 ,0x00                               // Sampling Rate(Lsb first). 
    ,0x00 ,0x00 ,0x00                               // Future Use 
    ,0xB5 ,0x01                                     // Store and forward Data aging(Lsb first) (437)base10. 
    ,0x00 ,0x00                                     // Math Notif.ratio (Lsb first).
    };

/*****************************************************************
 *Example 6 :                                                    *
 *  Data Acq.mode            : commisionning.                    *
 *                                                               * 
 ****************************************************************/
unsigned char a_DataAcqConfigOtacEX6[OTAC_LENGTH] = {
     0xa4 ,0xd5 ,0x78 ,0x43 ,0xde ,0xdd ,0x00 ,0x00 
    ,0x17 
    ,0x10 
    ,0x01 
    ,0x20 ,0x00                                       
    ,0x00 ,0x00 
    ,0x00 ,0x00 ,0x00 
    ,0x00 
    ,0x00 ,0x00 ,0x00 
    ,0x00 ,0x00 ,0x00 
    ,0x00 ,0x00 ,0x00 
    ,0x00 ,0x00 
    ,0x00 ,0x00
    };

/*****************************************************************
 *Example 7 :                                                    *
 *  Data Acq.mode            : Set.                              *
 *  Sampling Rate            : 10 Hz.                            *
 *  Data acquisition mode    : TX only.                          *
 *  Streaming Packet Options : Burst.                            *
 *  Notif.cycle              :  10 min.                          *
 *  Data Acq.duration        :  20s.                             *
 *  Store and Forward        : No.                               *
 *  |-->Data Aging           : No.                               *    
 *  Stand Alone              : No.                               *  
 ****************************************************************/
unsigned char a_DataAcqConfigOtacEX7[OTAC_LENGTH] = {
     0xa4 ,0xd5 ,0x78 ,0x43 ,0xde ,0xdd ,0x00 ,0x00 // The Mac Id (Msb First).   
    ,0x17                                           // The OTAC Length is (23)Decimal {default}.
    ,0x10                                           // The OTAC Id  (0x10)Hex Default.
    ,0x05                                           // The Daq Mod S.E.T. 
    ,0x20 ,0x00                                     // 0b00100000 only tx enabled.
    ,0x00 ,0x00                                     // Future Use 
    ,0x58 ,0x02 ,0x00                               // Daq duty cycle(Lsb first) Little Endian (600)second in Decimal.
    ,0x00                                           // TX Ratio.   
    ,0x14 ,0x00 ,0x00                               // Daq duration(Lsb first) Little Endian (20)second in Decimal.
    ,0x0a ,0x00 ,0x00                               // Sampling Rate(Lsb first) 10hZ.
    ,0x00 ,0x00 ,0x00                               // Future Use 
    ,0x00 ,0x00                                     // Store and forward Data aging(Lsb first).  
    ,0x00 ,0x00                                     // Math Notif.ratio (Lsb first).
};

/*****************************************************************
 *Example 8 :                                                    *
 *  Data Acq.mode            : Streaming.                        *
 *  Sampling Rate            : 10 Hz.                            *
 *  Data acquisition mode    : TX.                               *
 *  Streaming Packet Options : one shot.                         *
 *  Data Acq.duration        : 9min.                             *
 *  Store and Forward        : No.                               *
 *  |-->Data Aging           : No.                               *    
 *  Stand Alone              : No.                               *  
 ****************************************************************/
unsigned char a_DataAcqConfigOtacEX8[OTAC_LENGTH]={
    0xa4 ,0xd5 ,0x78 ,0x43 ,0xde ,0xdd ,0x00 ,0x00  // The Mac Id (Msb First). 
    ,0x17                                           // The OTAC Length is (23)Decimal {default}.
    ,0x10                                           // The OTAC Id  (0x10)Hex {Default}.
    ,0x03                                           // The Daq Mod Streaming.    
    ,0x28 ,0x00                                     // 0b00101000 only tx enabled ,(0b010)binary Streming one shot.
    ,0x00 ,0x00                                     // Future Use  
    ,0x00 ,0x00 ,0x00                               // Daq duty cycle(Lsb first).
    ,0x00                                           // TX Ratio. 
    ,0x1c ,0x02 ,0x00                               // Daq duration(Lsb first) Little Endian (540)second in Decimal.                         
    ,0x0a ,0x00 ,0x00                               // Sampling frequency. 10hz
    ,0x00 ,0x00 ,0x00                               // Future Use 
    ,0x00 ,0x00                                     // Store and forward Data aging(Lsb first).  
    ,0x00 ,0x00                                     // Math Notif.ratio (Lsb first).        
    };

/*pointers to each buffer contain the packet of OTAC*/
unsigned char (*p_DataAcqConfigOtacEX1)[OTAC_LENGTH] = &a_DataAcqConfigOtacEX1;

unsigned char (*p_DataAcqConfigOtacEX2)[OTAC_LENGTH] = &a_DataAcqConfigOtacEX2;

unsigned char (*p_DataAcqConfigOtacEX3)[OTAC_LENGTH] = &a_DataAcqConfigOtacEX3;

unsigned char (*p_DataAcqConfigOtacEX4)[OTAC_LENGTH] = &a_DataAcqConfigOtacEX4;

unsigned char (*p_DataAcqConfigOtacEX5)[OTAC_LENGTH] = &a_DataAcqConfigOtacEX5;

unsigned char (*p_DataAcqConfigOtacEX6)[OTAC_LENGTH] = &a_DataAcqConfigOtacEX6;

unsigned char (*p_DataAcqConfigOtacEX7)[OTAC_LENGTH] = &a_DataAcqConfigOtacEX7;

unsigned char (*p_DataAcqConfigOtacEX8)[OTAC_LENGTH] = &a_DataAcqConfigOtacEX8;


int main(int argc, char* argv[])
{
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    int rc;

    MQTTClient_create(&client, ADDRESS, CLIENTID,MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 19;
    conn_opts.cleansession = 1;

    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        exit(-1);
    }
    pubmsg.payload = p_DataAcqConfigOtacEX8 ;  // Change {p_DataAcqConfigOtacEX?} by the number of the packet
                                               // example{1-->7} did you want to send to the beanair device .

    pubmsg.payloadlen = OTAC_LENGTH;           // Set the palyload length.
    pubmsg.qos        = QOS;                   // Set the quaity of service.
    pubmsg.retained   = 0;                     // We don't use the retain flag.
    MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token);
    rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
    printf("Message with delivery token %d delivered\n", token);
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return rc;
}
