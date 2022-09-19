/******************************************************************************************************************************
 *
 * Company    : BeanAir
 * Date       : 09/05/2022
 * Author     : Marwene mechri
 *
 *Description:
 *This Example uses MQTT protocol to collect data measurments from a BeanDevice in streaming mode,
 *then display it the screen console.The beandevice used is "H-Inc" with 2 channels active ,
 *and the local broker ip is "192.168.1.91", so make sure to:
 *   + set your configuration right before running the example.
 *   + connect this program and the BeanDevice to the same broker.
 *Informations we need:
 *   Streaming frame content:
 * +(0)---------+(1)------+(2)-------+(8)----------------+(10)------------+(14)--------------++
 * |Device type |DAC type |time      |sampling frequency |channels bitmap |frame sequence id ||
 * |(1 byte)    |(1 byte) |(6 bytes) |(2 bytes)          |(4 bytes)       |(3 bytes)         ||
 * +------------+---------+----------+-------------------+----------------+------------------++
 * +(17)-----------------------+(19)------+(22)---------+(25)--------------------------------++
 * |number of data per channel |DAC cycle |DAC duration |previouse number of dac per channel ||
 * |(2 bytes)                  |(3 bytes) |(3 bytes)    |(2 bytes)                           ||
 * +---------------------------+----------+-------------+------------------------------------++
 * +(27)-----+(28)-----+(29)-----+(29+n)
 * |flags    |LOI      |Data     |
 * |(1 byte) |(1 byte) |(n bytes)|  
 * |---------+---------+---------+
 * 
 * 
 *   Time:
 * +---------------+----------------------+
 * |Reference time |Reference millisecond | 
 * |(4 bytes)      |(2 bytes)             |
 * +---------------+----------------------+
 *   Flags field:
 * +--------------- +-----------+
 * |Synchronization |future use | 
 * |(1 bit)         |(7 bits)   |
 * +----------------+-----------+
 * 
 *    Data field:
 * <-------------------------- sample 1 of each active channel ---------------------------------->
 * +----------------------------+----------------------------+------+----------------------------+
 * |Data sample 1 for channel 1 |Data sample 1 for channel 2 | .... |Data sample 1 for channel n |
 * |(3 bytes)                   |(3 bytes)                   | .... |(3 bytes)                   |
 * +----------------------------+----------------------------+------+----------------------------+
 * 
 * <--------------- sample 2 of each active channel ------------------->       
 * ----------------------------+----+----------------------------+-----+
 * Data sample 2 for channel 1 | .. |Data sample 2 for channel n | ... |
 * (3 bytes)                   | .. |(3 bytes)                   | ... |
 * ----------------------------+----+----------------------------+-----|
 * 
 * <--------------- sample m of each active channel --------------------->
 * +----+-----------------------------+----+-----------------------------+
 * |... | Data sample m for channel 1 | .. | Data sample m for channel n |
 * |... | (3 bytes)                   | .. | (3 bytes)                   |
 * |----+-----------------------------+----+-----------------------------+
 * 
 * 
 * (Note: Data sample in 3 bytes, the last bit is a sign bit and all field are LSB first)
 * Reference
 * docuement http://www.wireless-iot.beanair.com/files/TN-RF-004-MQTT-Comnmunication-Protocol.pdf (pages: 51, 51)
 * Preparation:
 * 1- Configure the beandevice to use the MQTT mode
 * 2- Make sure to enable the topic for dynamic measurement
 * 3- Start running the beandevice in Streaming mode
 * Steps:
 * 1- connect to the broker
 * 2- subscribe to the topic "STREAMING" which is the topic for the dynamic measurement
 * 3- wait for payloads
 * 4- check DAC type if it's streaming
 * 5- parse the coming payload
 * 6- display data in graph
'''*/


/*
####################################
############# Libraries ############
####################################
*/

/****** Standards library of c language ******/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/****** Paho MQTT C Client library ***********/ 
#include "MQTTClient.h"

  /*
  ####################################
  ########### Configuration ##########
  ####################################
  */

/****** Broker config ****************/
#define  PORT_BROKER     "1883"                                    /*Change the PORT_BROKER with the port number of your broker.*/
#define  ADDRESS_BROKER  "192.168.1.91"                            /*Change the ADDRESS_BROKER with the IP OR HOSTNAME of your broker.*/
#define  CLIENTID        "ExampleClientSubBeanair"
#define  TIMEOUT         1000L
#define  QOS             1

/****** Beandevice Configuration *****/
#define DEVICE_MAC_ID    "A4D57843DEDD0000"                        /*Change the DEVICE_MAC_ID with the mac id of your device*/

/****** Global variables *************/
#define  TOPIC             DEVICE_MAC_ID "/STREAMING"
#define  ADDRESS           ADDRESS_BROKER ":"  PORT_BROKER
#define  TRUE              1
#define  OTAC_LENGTH       2000                                     /*The length of buffer that contains the packet.*/
unsigned char  Otac_acq[OTAC_LENGTH];                               /*The buffer that contains the packet.*/

//**@Define type of variables.**/
typedef struct s_Date
{
  unsigned long long      m_year;
  unsigned char           m_month;
  unsigned char           m_day;
  unsigned char           m_hour;
  unsigned char           m_minute;
  unsigned char           m_second;
  unsigned long long      m_UnixFormat;
} t_Date;

typedef struct  s_StreamingBurstPacket
{
    const char *m_deviceType ;
    unsigned char m_acquisitionMode;
    t_Date        m_streamingDate;
    unsigned long long m_refMilliSecond;
    unsigned long long m_sampligFreq;
    unsigned long long m_channelsBitMap;
    unsigned long long m_frameSequenceId;
    unsigned long long m_numberAcqPerChannel;
    unsigned long long m_dataAcqCycle;
    unsigned long long m_dataAcqDuration;
    unsigned long long m_preNumberAcqPerChannels;
    unsigned char      m_flags;
    unsigned char      m_networkQuality;
    unsigned char      m_numberactivatedCahnnels
} t_StreamingPacket;

typedef enum StreamingBurstacket{DEVICE_TYPE=0,ACQ_TYPE,REF_TIME_UNIX,REF_MILLISECONDS=6,SAMPLING_FREQ=8,CHANNELS_BIT_MAP=10,
                                 FRAME_SEQUENCE_ID=14,NUMBER_ACQ_PER_CHANNELS=17,DATA_ACQ_CYCLE=19,DATA_ACQ_DURATION=22,
                                 PREVIOUS_NUMBER_ACQ_PER_CHANNEL=25,FLAG=27,NETWORK_QUALITY,DATA_SAMPLE} StreamingBurstacket_t; 

const char a_DeviceType[7][13] = { "NN" ,"AX_3D"  ,"HI_INC_MONO" ,"HI_Inc_BI" ,"X_INC_MONO" ,"X_INC_BI" ,"AX3DS"  };
typedef  enum { NN, AX_3D ,HI_INC_MONO ,HI_Inc_BI ,X_INC_MONO ,X_INC_BI ,AX3DS  } DeviceType_t;
typedef enum  {LOW_DUTY_CYCLE=0X01 ,ALARM=0X02 ,STREAMING=0X03 ,SHOCK_DETECTION=0X04 ,LDC_MATH_RESULT=0X05 ,S_E_T=0X06 ,DYNAMIC_MATH_RESULT=0X07} AcqType_t;


/**@Function of Beanair MQTT C CLIENT declaration .**/
const char *vDetectDeviceType(const unsigned char deviceType);
void vFrameSequence(const unsigned char * const Otac,unsigned char LENGTH, unsigned long long *p_frameSequence);
void vNumAcqPerChannel(const unsigned char *const Otac,const unsigned char LENGTH, unsigned long long *p_AcqPerChannel);
void vDataAcqCycle(const unsigned char * const Otac ,const unsigned char LENGTH ,unsigned long long *p_AcqCycle );
void vDataAcqDuration(const unsigned char * const  Otac ,const unsigned char LENGTH ,unsigned long long *p_AcqDuration );
void vPreviousNumberOfData(const unsigned char *const Otac ,const unsigned char LENGTH ,unsigned long long *p_PreviousNumberOfData);
t_Date unixTimeToHumanReadable(unsigned long long  seconds);
void vReferenceTime(const unsigned char * const Otac,const unsigned char LENGTH ,unsigned long long *p_refMillis);
void vDisplayDacMode(const unsigned char  *Otac);
unsigned char uchDetectAcqmode(const unsigned char BenDevice);
void vDetectSamplingFrequency(const unsigned char *Otac, const unsigned char LENGTH, unsigned long long *p_SamplingFreq );
void vDetectActivatedChannels(const unsigned char *Otac ,const unsigned char LENGTH,unsigned long long  * p_ActivatedChannels);
void vDateUnixHexToDec(const unsigned char * unixDate ,const unsigned char LENGTH ,t_Date *s_Date);
void VDetectDataFromFrame(const unsigned char *p_INPUT ,unsigned char *p_output,const unsigned char  LENGTH,StreamingBurstacket_t byteIndex);
void vConcatenateBytes(const unsigned  char * a_Bytes ,unsigned long long *concatenatedBytes ,const unsigned char LENGTH_BUFFER_BYTES );
void vCalculateSubPack( const t_StreamingPacket *const p_StreamingPacket);
void vDetectNetworkQuality(const unsigned char * a_Otac ,unsigned char * p_NetworkQuality );
void vDetectFlags(const unsigned char * a_Otac ,unsigned char * p_Flags );

/*___Function of Paho MQTT C Client Library___*/
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
    //printf("   message: ");
    payloadptr = message->payload;
   //printf("\nthe lenght of the buffer %d",message->payloadlen );
    for(i=0; i<message->payloadlen; i++)
    {
        //putchar(*payloadptr++);
        Otac_acq[i]=*payloadptr++;

        //printf("0x%02X ,",*payloadptr++);
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

/*___The main function___*/
int main(int argc, char* argv[])
{
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    int rc;
    int ch;
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
    printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n"
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

void vDisplayDacMode(const unsigned char   *Otac)
{
  /***
   *@Receive Streaming acquisition data generated by the device by The MQTT protocols and 
   *store the data in the tStreamingPacket data type to slice the data of the packet.
   ***/
  t_StreamingPacket s_streamingPacket;

  s_streamingPacket.m_acquisitionMode= uchDetectAcqmode(Otac[ACQ_TYPE]);  //detect_Acq_mode 
  if(s_streamingPacket.m_acquisitionMode==STREAMING)
  {
    /**Slicing the data come from the /Streaming  topic.**/
    printf("#######################################################################");
    printf("\n  Message arrived\n");
    printf("  Topic: %s\n",TOPIC);
    printf("\nData Acq.mode               :Streaming");

    s_streamingPacket.m_deviceType  =vDetectDeviceType(Otac[DEVICE_TYPE]);x²  

    vDateUnixHexToDec(Otac + REF_TIME_UNIX ,4 ,&(s_streamingPacket.m_streamingDate));

    vReferenceTime(Otac + REF_MILLISECONDS ,2 ,&(s_streamingPacket.m_refMilliSecond));

    vDetectSamplingFrequency(Otac+SAMPLING_FREQ,2,&(s_streamingPacket.m_sampligFreq));

    vDetectActivatedChannels(Otac+CHANNELS_BIT_MAP,4,&(s_streamingPacket.m_channelsBitMap));

    vFrameSequence(Otac+FRAME_SEQUENCE_ID,3,&(s_streamingPacket.m_frameSequenceId));     

    vNumAcqPerChannel(Otac+NUMBER_ACQ_PER_CHANNELS,2,&(s_streamingPacket.m_numberAcqPerChannel)); 
    
    vDataAcqCycle(Otac+DATA_ACQ_CYCLE , 3 , &(s_streamingPacket.m_dataAcqCycle));  

    vDataAcqDuration(Otac+DATA_ACQ_DURATION,3,&(s_streamingPacket.m_dataAcqDuration)); 

    vPreviousNumberOfData(Otac+PREVIOUS_NUMBER_ACQ_PER_CHANNEL ,2 ,&(s_streamingPacket.m_preNumberAcqPerChannels ));
  
    vDetectFlags(Otac+FLAG , &(s_streamingPacket.m_flags));

    vDetectNetworkQuality(Otac+NETWORK_QUALITY , &(s_streamingPacket.m_networkQuality));

    vCalculateSubPack(&s_streamingPacket);
 }


}
const char *vDetectDeviceType(const unsigned char deviceType)
{
  /**@Detect the device type and print it in the console.**/
switch (deviceType)
  { 
    case AX_3D:
      printf("\nDevice Type                 :AX_3D device.");   
      return  a_DeviceType [AX_3D]; 
    case HI_INC_MONO:
      printf("\nDevice Type                 :HI_INC_MONO Device.");  
      return  a_DeviceType [HI_INC_MONO];  
    case HI_Inc_BI:
      printf("\nDevice Type                 :HI_Inc_BI Device.");
      return  a_DeviceType [HI_Inc_BI]; 
    case X_INC_MONO:
      printf("\nDevice Type                 :X_INC_MONO Device.");  
      return  a_DeviceType [X_INC_MONO];   
    case X_INC_BI:
      printf("\nDevice Type                 :X_INC_BI Device.");
      return  a_DeviceType [X_INC_BI]; 
    case AX3DS:
      printf("\nDevice Type                 :AX3DS device.");
      return  a_DeviceType [AX3DS]; 
    default:
      printf("\nDevice Type                 :NN");
      return  a_DeviceType [NN]; 
  }
}
unsigned char  uchDetectAcqmode(const unsigned char BenDevice)  //detect_Acq_mode
{
  switch (BenDevice)
  {
    case LOW_DUTY_CYCLE:
      return LOW_DUTY_CYCLE;

    case ALARM:
      return ALARM;

    case STREAMING:
      return STREAMING;

    case SHOCK_DETECTION:
      return SHOCK_DETECTION;
        
    case LDC_MATH_RESULT:
      return LDC_MATH_RESULT; 

    case S_E_T:
      return S_E_T;

    case DYNAMIC_MATH_RESULT:
      return DYNAMIC_MATH_RESULT;

    default:
      return NN;
  }
}

 void vDetectActivatedChannels(const unsigned char *Otac ,const unsigned char LENGTH,unsigned long long * p_ActivatedChannels)
{
  /***
   *@Detect the Activated channels.
   ***/
  unsigned long long  channelBitMapMask=1;
  unsigned char channelsActivatedNumber=0;

  vConcatenateBytes(Otac,p_ActivatedChannels ,LENGTH);
  for (int channelBitMapIndex=0 ;channelBitMapIndex<32; channelBitMapIndex++)
  {
    if(*p_ActivatedChannels & channelBitMapMask)
    {
        channelsActivatedNumber++;
    }
    channelBitMapMask<<=1;
  }
  *p_ActivatedChannels=channelsActivatedNumber;
  printf("\nThe activated number is     : %llu",*p_ActivatedChannels);
}

void vDetectSamplingFrequency(const unsigned char *Otac, const unsigned char LENGTH, unsigned long long *p_SamplingFreq )
  {
    /***
     *@Detect the sampling frequency field and print it on console.
     ***/

      vConcatenateBytes(Otac,p_SamplingFreq ,LENGTH);
      printf("\nsampling frequency          :%lld",*p_SamplingFreq);   
  }

void vReferenceTime(const unsigned char * const Otac ,const unsigned char LENGTH ,unsigned long long *p_refMillis)
{
  /***
   *@Detect the reference time field and print it on console.
   ***/

  vConcatenateBytes(Otac ,p_refMillis ,LENGTH);
  printf("\nReference time              :%lld  ",*p_refMillis);
}






void vFrameSequence(const unsigned char * const Otac, const unsigned char LENGTH, unsigned long long *p_frameSequence)
{
 /***
  *@Detect the frame sequence field and print it on the console.
  ***/

  vConcatenateBytes(Otac,p_frameSequence ,LENGTH);
  printf("\nThe Frame sequence Id       :%lld  ",*p_frameSequence);
}


void vNumAcqPerChannel(const unsigned char *const Otac,const unsigned char LENGTH, unsigned long long *p_AcqPerChannel)
{
  /***
   *@Detect the Number of acquisition per channels field and print it on the console.
   ***/

  vConcatenateBytes(Otac,p_AcqPerChannel ,LENGTH);  
  printf("\nThe Num_Acq_per_channel     :%lld  ",*p_AcqPerChannel);
}

void vDataAcqCycle(const unsigned char * const Otac ,const unsigned char LENGTH ,unsigned long long *p_AcqCycle )
{
  /***
   *@Detect Data acquisition cycle field and print it on the console.
   ***/

  vConcatenateBytes(Otac,p_AcqCycle ,LENGTH);  
  if(*p_AcqCycle==0)
  {
    printf("\nData Acq.cycle    :NA "  );
  }
  else 
  {
  if(*p_AcqCycle/3600>24)
    {
      printf("\nData Acq.cycle              :%02lld:%02lld:%02lld:%02lld ",*p_AcqCycle/(24*3600),(*p_AcqCycle%(24*3600))/3600,(*p_AcqCycle%3600)/60,*p_AcqCycle%60   );
    }
  else
    {
      printf("\nData Acq.cycle              :%02lld:%02lld:%02lld ",*p_AcqCycle/3600,(*p_AcqCycle%3600)/60,*p_AcqCycle%60   );
    }
  }
}


void  vDataAcqDuration(const unsigned char * const  Otac ,const unsigned char LENGTH ,unsigned long long *p_AcqDuration )
{
  /***
   *@Detect the Data acquisition duration field and print it on the console.
   ***/
  vConcatenateBytes(Otac, p_AcqDuration,LENGTH);  
  if(*p_AcqDuration==0)
  {
    printf("\nData Acq.duration           :NA "  );
  }
  else
  {
    if(*p_AcqDuration/3600>24)
    {
      printf("\nData Acq.duration           :%02lld:%02lld:%02lld:%02lld ",*p_AcqDuration/(24*3600),(*p_AcqDuration%(24*3600))/3600,(*p_AcqDuration%3600)/60,*p_AcqDuration%60   );
    } 
    else
    {
      printf("\nData Acq.duration           :%02lld:%02lld:%02lld ",*p_AcqDuration/3600,(*p_AcqDuration%3600)/60,*p_AcqDuration%60   );          
    }
  }
}

void vPreviousNumberOfData(const unsigned char *const Otac ,const unsigned char LENGTH ,unsigned long long *p_PreviousNumberOfData)
{
  /***
   *@Detect the previous number of data field and print it on the console.
   ***/

  vConcatenateBytes(Otac,p_PreviousNumberOfData,2);
  printf("\nThe Previous_number_of_data :%llu ",*p_PreviousNumberOfData);
}

void VDetectDataFromFrame(const unsigned  char *p_INPUT ,unsigned char *p_output,const unsigned char  LENGTH,StreamingBurstacket_t byteIndex)
{
   /***
    *@Detect the data from the frame
    ***/
    unsigned char temp=0;
    for(int currentItemIndex=0;currentItemIndex<LENGTH;currentItemIndex++)
    {
        temp=p_INPUT[byteIndex+currentItemIndex];
        p_output[currentItemIndex]=temp;
    }
}

void vDateUnixHexToDec(const unsigned char * unixDate ,const unsigned char LENGTH ,t_Date *s_Date)
{
    /***
     *@CONVERT THE HEX UNIX DATA FORMAT TO DECIMAL 
     **/

    vConcatenateBytes(unixDate ,&(s_Date->m_UnixFormat) ,LENGTH );
}

void vConcatenateBytes(const unsigned  char * a_Bytes ,unsigned long long *concatenatedBytes ,const unsigned char LENGTH_BUFFER_BYTES )
{
   /***
    *@Concatenate Bytes array of bytes {Msb in the first index of the array "a_Bytes"}. 
   ***/
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

t_Date unixTimeToHumanReadable(unsigned long long  seconds)
{
    /***
     * @Convert  the Unix Unix timestamp to a human-readable date format.
     ***/

    t_Date  dateAcq;
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

    printf("\nThe date                    :%ld",date);
    dateAcq.m_day=date;
    dateAcq.m_month=month;
    printf("/%ld",month);
    dateAcq.m_year=currYear;
    printf("/%02ld ",currYear);
    dateAcq.m_hour=hours;
    printf("%02ld",hours); 
    printf(":%02ld",minutes);
    printf(":%02ld",secondss);
    dateAcq.m_minute=minutes;
    dateAcq.m_second=secondss;
    dateAcq.m_UnixFormat=seconds;
    printf("\nUnix time: %lld s",seconds);
    // Return the time
    return dateAcq;
}

void vCalculateSubPack( const t_StreamingPacket *const p_StreamingPacket)   
{
  /***
   * @Detect the data sampling for each channel and the time sub_packet 
   * ,then print it  in the console.
   ***/
   
  double tSubPacket=0;
  long long subPacketIndex=0;
  double dataSample=0;
  for(int current_subPacket_row=0;current_subPacket_row<p_StreamingPacket->m_numberAcqPerChannel;current_subPacket_row++)
  { 
    printf("\nThe index Items %d",current_subPacket_row);
    subPacketIndex=((p_StreamingPacket->m_frameSequenceId)*(p_StreamingPacket->m_preNumberAcqPerChannels))+current_subPacket_row;  
    tSubPacket=(p_StreamingPacket->m_streamingDate.m_UnixFormat)+((double)p_StreamingPacket->m_refMilliSecond/1000)+((1/(double)p_StreamingPacket->m_sampligFreq) *subPacketIndex); 
    printf("\n---------------------/ / / / / /----------------------------------------");
    printf("\nThe subPacket Time          :%lf ",tSubPacket  );
    printf("\nThe subPacket Index         :%lld",subPacketIndex);
  
    unsigned long long dataSampleAfterCon=0;
    for(int channelIndex=0 ;channelIndex< p_StreamingPacket->m_channelsBitMap ; channelIndex++)
    {
      vConcatenateBytes(((Otac_acq+DATA_SAMPLE+(channelIndex*3))+(3*p_StreamingPacket->m_channelsBitMap*current_subPacket_row)),&dataSampleAfterCon ,3 );
      dataSample=(dataSampleAfterCon&0x7fffff);
      dataSample/=1000;
      if(dataSampleAfterCon & 0x800000)
      {
        dataSample*=(-1);
      }
      printf("\nData sample measured        :%f",dataSample);
    }
  }
}

void vDetectNetworkQuality(const unsigned char * a_Otac ,unsigned char * p_NetworkQuality )
{
  /**@Detect the network quality field and display it in the screen.**/
   *p_NetworkQuality=*a_Otac;
   printf("\nThe Network quality         :%d",*p_NetworkQuality);
}

void vDetectFlags(const unsigned char * a_Otac  ,unsigned char * p_Flags )
{
  /**@Detect the flags fileld and display it in the screen.**/
  *p_Flags=*a_Otac;
  printf("\nThe flags                   :%d",*p_Flags);
}