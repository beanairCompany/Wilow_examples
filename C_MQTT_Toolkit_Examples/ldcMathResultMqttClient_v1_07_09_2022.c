/******************************************************************************************************************************
 *
 * Company    : BeanAir
 * Date       : 09/05/2022
 * Author     : Marwene mechri
 *
 * Description:
 * This Example uses MQTT protocol to receive, parse and display the LDC math result.
 * The beandevice used is "H-Inc" with 2 channels active and the local broker ip is "192.168.1.91", so make sure to:
 *    + set your configuration right before running the example.
 *    + connect this program and the BeanDevice to the same broker.
 * Informations we need:
 *    LDC (Low Duty Cycle) Math Result frame content:
 * +(0)---------+(1)------+(2)--------+(3)------------+(4)--------+(8)--------+(12)------+(16)--------+(19)
 * |Device type |DAC type |Channel id |Math result Id |Event time |Start time |End time  |Data sample |
 * |(1 byte)    |(1 byte) |(1byte)    |(1 byte)       |(4 bytes)  |(4 bytes)  |(4 bytes) |(3 bytes)   |
 * +------------+---------+-----------+---------------+-----------+-----------+----------+------------+
 * (Note: 
 *    + Time (Event, Start and End) is in Unix format (LSB first)
 *    + Data sample (LSB first), the last bit is a sign bit)
 *    + The Event time is for the maximum and minimum math result only
 *    + The Start and End time is for the average math result only
 * Reference
 * docuement http://www.wireless-iot.beanair.com/files/TN-RF-004-MQTT-Comnmunication-Protocol.pdf (pages: 56, 57)
 * Preparation:
 * 1- Configure the beandevice to use the MQTT mode
 * 2- Make sure to enable the topic for static measurement of each channel
 * 3- Start running the beandevice in LDC mode
 * Steps:
 * 1- connect to the broker
 * 2- subscribe to Ldc math result topics which is the list of static measurement topics in the beanscape MQTT configuration of the beandevice {[MAC_ID]/Sensor0, [MAC_ID]/Sensor1 ... etc}
 * 3- wait for payloads
 * 4- check DAC type if it's LDC Math Result
 * 5- parse the coming payload
 * 6- print the information
 *
 ***********************************************************************************************************************************************************************************************************************/
/*
####################################
############# Libraries ############
####################################
*/

/******standards library of c language******/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// Paho MQTT C Client Library 
#include "MQTTClient.h"

/*
####################################
########### Configuration ##########
####################################
*/
// broker config
#define  PORT_BROKER     "1883"
#define  ADDRESS_BROKER  "192.168.1.91"                       /*Change the ADDRESS_BROKER with the IP OR HOSTNAME of your broker.*/
#define  CLIENTID        "ExampleClientSubBeanair"
#define  TIMEOUT         1000L

/* beandevice */
#define DEVICE_MAC_ID    "A4D57843DEDD0000"                   /*Change the DEVICE_MAC_ID with the mac id of your device*/

/*NOTE:
 * @all topics should be set in the beandevice using the beanscape software (select device -> click BeanDevice -> MQTT)
 * @The static measurment create one topic for each channel, so this list will store the topic with the corresponding channel name
 * @make sure to subscripe to an existing topic, mean the cahnnel is active.

#define  TOPIC             DEVICE_MAC_ID "/SENSOR/+"

/****
 * @Subscription wildcards 
 * @A '+' character represents a single level of the hierarchy and is used between delimiters.
 * @For example, SENSOR/1 will match SENSOR/2 and SENSOR/3 SENSOR/4 SENSOR/5.
 ****/

#define  ADDRESS         ADDRESS_BROKER ":"  PORT_BROKER
#define  QOS         1
#define  OTAC_LENGTH 11 
#define  TRUE 1

typedef enum Math_Result_Id{MAXIMUM=0X01,MINIMUM=0X02,AVERAGE=0X03}Math_Result_Id_t;
typedef enum Daq_Mode{LOW_DUTY_CYCLE=1, ALARM ,LOW_DUTY_CYCLE_MATH_RESULT=5 } Daq_Mode_t;
typedef enum Device_Type{ X_3D=1, HI_INC_MONO, HI_INC_BI, X_INC_MONO, X_INC_BI, AX3DS} Device_Type_t;
typedef enum Packet_Ldc_Math {DEVICE_TYPE,DAC_TYPE ,CHANNEL_ID ,MATH_RESULT_ID ,EVENT_IN_UNIX_TIME_FORMAT 
             ,START_IN_UNIX_TIME_FORMAT=8 ,END_IN_UNIX_TIME_FORMAT=12,DATA_SAMPLE_LDC_MATH=16} Packet_Ldc_Math_t;
             
typedef struct s_Date
{
 unsigned char     m_day;
 unsigned char     m_month;
 unsigned short    m_year;
 unsigned char     m_hour;
 unsigned char     m_minute;
 unsigned char     m_second;
} t_Date;


unsigned char u8DetectDeviceType(const unsigned char BenDevice);
unsigned char  uchDetectAcqmode(const char *  BenDevice);
unsigned char u8detect_channel_id(unsigned char channel_id);
double dDataSampleMeasured(const unsigned char *data_measured);
void vDisplayDacMode(const char  *Otac);
t_Date * unixTimeToHumanReadable(long int seconds);
void vDateUnixHexToDec(const unsigned char *unixDate);
void VDetectDataFromFrame(const char *p_INPUT ,unsigned char *p_output,const unsigned char  LENGTH,Packet_Ldc_Math_t byteIndex);
void vConcatenateBytes(const unsigned  char * a_Bytes ,unsigned long long *concatenatedBytes ,const unsigned char LENGTH_BUFFER_BYTES );
unsigned char uchDetectMathResultsId(const char *p_Otac  );




char  Otac_acq[OTAC_LENGTH];
volatile MQTTClient_deliveryToken deliveredtoken;
void delivered(void *context, MQTTClient_deliveryToken dt)
{
    printf("Message with token value %d delivery confirmed\n", dt);
    deliveredtoken = dt;
}
int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    int i;
    char* payloadptr;
    payloadptr = message->payload;
    for(i=0; i<message->payloadlen; i++)
    {
        Otac_acq[i]=*payloadptr++;
#ifdef DEBUG 
        /* display the received frame */
         printf("0x%02X ,",Otac_acq[i]);
#endif
    }
    vDisplayDacMode(Otac_acq);
    putchar('\n');
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}
void connlost(void *context, char *cause)
{
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
}
int main(int argc, char* argv[])
{
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    char  rc;
    char  ch;
    MQTTClient_create(&client, ADDRESS, CLIENTID,
        MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }
    printf("Subscribing to topic %s\nfor client %s us ing QoS%d\n\n"
           "Press Q<Enter> to quit\n\n", TOPIC, CLIENTID, QOS);
    MQTTClient_subscribe(client, TOPIC, QOS);
    do
    {
        ch = getchar();
    } while(ch!='Q' && ch != 'q');
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return rc;
}

void vDisplayDacMode(const char  *Otac)
{   /**@Slicing the upcoming frame**/
    unsigned char dataAqcType=-1;
    unsigned char a_DateUnixFormat[4];
    unsigned char dataSample[3];
    dataAqcType=uchDetectAcqmode(Otac+DAC_TYPE); 
    if(dataAqcType==LOW_DUTY_CYCLE_MATH_RESULT)
    {
        /**@Collect data measurments from a BeanDevice of Low_duty_Math_Result */
        printf("**************************************************************************\n");
        printf("Message arrived\n");
        printf("\nData Acq.mode       :Low Duty Cycle");
        unsigned char channel_id =u8detect_channel_id(Otac[CHANNEL_ID]);
        unsigned char deviceType =u8DetectDeviceType(Otac[DEVICE_TYPE]);
        unsigned char mathResults=0;
        mathResults=uchDetectMathResultsId(Otac);
        if(mathResults==MAXIMUM)
        {
            VDetectDataFromFrame(Otac ,a_DateUnixFormat ,4,EVENT_IN_UNIX_TIME_FORMAT);
            vDateUnixHexToDec(a_DateUnixFormat);
            printf("            Time of Maximum Data sample measured event. ");
        }
        if(mathResults==MINIMUM)
        {
            VDetectDataFromFrame(Otac ,a_DateUnixFormat ,4,EVENT_IN_UNIX_TIME_FORMAT);
            vDateUnixHexToDec(a_DateUnixFormat);
            printf("            Time of Minimum Data sample measured event. ");
        }
        VDetectDataFromFrame(Otac ,dataSample,3,DATA_SAMPLE_LDC_MATH);
        double samlpeMesured=dDataSampleMeasured(dataSample);
        if(mathResults==AVERAGE)
        {   
            printf("                     Average data sample measured. ");
            VDetectDataFromFrame(Otac ,a_DateUnixFormat ,4,START_IN_UNIX_TIME_FORMAT);
            vDateUnixHexToDec(a_DateUnixFormat);
            printf("            Time of starting. ");
            VDetectDataFromFrame(Otac ,a_DateUnixFormat ,4,END_IN_UNIX_TIME_FORMAT);
            vDateUnixHexToDec(a_DateUnixFormat);
            printf("            Time of Ending. ");
        }
    }

}
unsigned char u8DetectDeviceType(const unsigned char BenDevice)
{
  /**@Detecting the device type and print it in screen**/
  switch (BenDevice)
     {
        case X_3D:
          printf("\nDevice Type         :AX_3D device.");
          return X_3D;
    	case HI_INC_MONO:
          printf("\nDevice Type         :HI_INC_MONO Device.");
      	  return HI_INC_MONO;
    	case HI_INC_BI:
          printf("\nDevice Type         :HI_Inc_BI Device.");
      	  return HI_INC_BI;
    	case X_INC_MONO:
          printf("\nDevice Type         :X_INC_MONO Device.");
      	  return X_INC_MONO;
    	case X_INC_BI:
      	  printf("\nDevice Type         :X_INC_BI Device.");
      	  return X_INC_BI;
    	case AX3DS:
      	  printf("\nDevice Type         :AX3DS device.");
      	  return AX3DS;
    	default:
      	  return -1;
     }
}
unsigned char  uchDetectAcqmode(  const  char *   BenDevice)
{
    /* detect the acquisition mode */
	switch (*BenDevice)
	  {
	    case LOW_DUTY_CYCLE:
	      return LOW_DUTY_CYCLE;
	    case ALARM:
	      return ALARM;
	    case LOW_DUTY_CYCLE_MATH_RESULT:
              return LOW_DUTY_CYCLE_MATH_RESULT;
	    default:
	      return -1;
	  }
	  return 0;
}
unsigned char  u8detect_channel_id(unsigned char channel_id)
{
    /*@Detect channel id and print it in screen.*/ 
    printf("\nChannel_id          :%d",channel_id);
    return channel_id;
}

t_Date * unixTimeToHumanReadable(long int seconds)
{
    /* Convert  the Unix Unix timestamp to a human-readable date format */
    static t_Date  dateAcq;
    // Number of days in month in normal year
    int daysOfMonth[] = { 31, 28, 31, 30, 31, 30,
                          31, 31, 30, 31, 30, 31 };

    long int currYear, daysTillNow, extraTime,
        extraDays, index, date, month, hours,
        minutes, secondss, flag = 0;
 
    // Calculate total days unix time T
    daysTillNow = seconds / (24 * 60 * 60);
    extraTime = seconds % (24 * 60 * 60);
    currYear = 1970;
 
    // Calculating current year
    while (daysTillNow >= 365) {
        if (currYear % 400 == 0
            || (currYear % 4 == 0
                && currYear % 100 != 0)) {
            daysTillNow -= 366;
        }
        else {
            daysTillNow -= 365;
        }
        currYear += 1;
    }
 
    // Updating extradays because it
    // will give days till previous day
    // and we have include current day
    extraDays = daysTillNow + 1;
 
    if (currYear % 400 == 0
        || (currYear % 4 == 0
            && currYear % 100 != 0))
        flag = 1;
 
    // Calculating MONTH and DATE
    month = 0, index = 0;
    if (flag == 1) {
        while (TRUE) {
 
            if (index == 1) {
                if (extraDays - 29 < 0)
                    break;
                month += 1;
                extraDays -= 29;
            }
            else {
                if (extraDays
                        - daysOfMonth[index]
                    < 0) {
                    break;
                }
                month += 1;
                extraDays -= daysOfMonth[index];
            }
            index += 1;
        }
    }
    else {
        while (TRUE) {
 
            if (extraDays
                    - daysOfMonth[index]
                < 0) {
                break;
            }
            month += 1;
            extraDays -= daysOfMonth[index];
            index += 1;
        }
    }
 
    // Current Month
    if (extraDays > 0) {
        month += 1;
        date = extraDays;
    }
    else {
        if (month == 2 && flag == 1)
            date = 29;
        else {
            date = daysOfMonth[month - 1];
        }
    }
 
    // Calculating HH:MM:YYYY
    hours = extraTime / 3600;
    minutes = (extraTime % 3600) / 60;
    secondss = (extraTime % 3600) % 60;

    printf("\nThe date            :%ld",date);
    dateAcq.m_day=date;
    dateAcq.m_month=month;
    printf("/%ld",month);
    dateAcq.m_year=currYear;
    printf("/%02ld ",currYear);
    dateAcq.m_hour=hours;
    printf("%02ld",hours); 
    printf(":%02ld",minutes);
    dateAcq.m_minute=minutes;
    dateAcq.m_second=seconds;
    printf(":%02ld",secondss);
    // Return the time
    return &dateAcq;
}

void vDateUnixHexToDec(const unsigned char * unixDate)
{
    /* __ CONVERT THE HEX UNIX DATA FORMAT TO DECIMAL __*/
    unsigned long long  dateUnixformaSecond=0;
    vConcatenateBytes(unixDate ,&dateUnixformaSecond ,4 );
#ifdef DEBUG  
   printf("data unix hex to  dec %lld",dateUnixformaSecond );
#endif
    t_Date * ts;
    ts  =   unixTimeToHumanReadable(dateUnixformaSecond);
}

double dDataSampleMeasured(const unsigned char *p_DataMeasured)
{
   /****@Detect the sample measured from the frame and print it in the screen 
    *@after reading "Data sample measured" field, the user must perform the following calculation:
    *@Decimal value =(-1)^(sign Bit)* (Remaining_bits_indecimal_format/ 1000). 
    ****/

    unsigned long long dataSampleAfterCon;
    double dataSample=0;
    vConcatenateBytes(p_DataMeasured,&dataSampleAfterCon ,3);
    dataSample=(dataSampleAfterCon&0x7fffff);
    dataSample/=1000;
    if(dataSampleAfterCon & 0x800000)
    {
        dataSample*=(-1);
    }
    printf("\nData sample measured:%f",dataSample);
    return dataSample;
}

void VDetectDataFromFrame(const char *p_INPUT ,unsigned char *p_output,const unsigned char  LENGTH,Packet_Ldc_Math_t byteIndex)
{
   /*Detect the data from the frame*/
    unsigned char temp=0;
    for(int currentItemIndex=0;currentItemIndex<LENGTH;currentItemIndex++)
    {
        temp=p_INPUT[byteIndex+currentItemIndex];
        p_output[currentItemIndex]=temp;
    }
}


void vConcatenateBytes(const unsigned  char * a_Bytes ,unsigned long long *concatenatedBytes ,const unsigned char LENGTH_BUFFER_BYTES )
{
   /****
    *@Concatenate Bytes array of bytes {Msb in the first index of the array "a_Bytes"}. 
   ****/
    *concatenatedBytes=0;
    unsigned long long bytesContainer=0;
    for (int itemsBuffer=(LENGTH_BUFFER_BYTES-1);itemsBuffer>=0;itemsBuffer-- )
    {
        bytesContainer=0;
        bytesContainer =     a_Bytes[itemsBuffer];
        bytesContainer <<=   ((itemsBuffer)*8);
        (*concatenatedBytes)+=bytesContainer;

    }
}

unsigned char uchDetectMathResultsId(const char *p_Otac  )
{
   /***
    *@Detect math results and print it in the screen .
    ***/

      printf("\nMath Result Id      :%d",p_Otac[3]);
      return p_Otac[3];
}
