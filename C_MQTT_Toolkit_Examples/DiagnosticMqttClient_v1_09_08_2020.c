/******************************************************************************************************************************
 *
 * Company    : BeanAir
 * Date       : 09/05/2022
 * Author     : Marwene mechri
 *
 * Description:
 * This Example uses MQTT protocol to collect diagnostic data  from a BeanDevice ,and display it in screen .   
 * The beandevice used is is "H-Inc" with 2 channels active and the local broker ip is "192.168.1.91" 
 * or you can use remote one like "broker.hivemq.com", so make sure to:
 *    + set your configuration right before running the example.
 *    + connect this program and the BeanDevice to the same broker.
 *  Informations we need:
 *    Dignostic frame content:
 * +(17)----+(19)----+(20)---+(21)---+(22)----+(23)----+(24)----+(25)----+(26)----+(28)----+(29)----+(31)--------+
 * |  Year  |  Month |  Day  |  Hour | Minute | Second |DiagType|reserved|   PER  |  LQI   |Reserved|Diag_Options|  
 * |(2 byte)|(1 byte)|(1byte)|(1byte)|(1byte) |(1 byte)|(1 byte)|(1 byte)|(2 byte)|(1 byte)|(2 byte)|  (2 byte)  |              
 * +--------+--------+-------+-------+--------+--------+--------+-----------------+--------+--------+------------+
 * +(33)---------------+(35)----+(37)-------------+(38)-----------+(39)----+(42)----------+(44)-------------------------+
 * |InternalTemperature|Reserved|DataloggerFreeMem|Energy_H_Status|Reserved|BatteryVoltage|Numb_OfAvailableSensorChannel|  
 * |      (2 byte)     | (2byte)|     (1byte)     |    (1byte)    |(3 byte)|   (2 byte)   |         (1 byte)            |    
 * +-------------------+--------+-----------------+---------------+--------+--------------+-----------------------------+
 * +(45)----------------++
 * | SensorStatusBitmap ||
 * |      (1 byte)      ||    
 * +--------------------++
 *
 * (Note:
 *   + Year ,PER ,Diag_Options ,InternalTemperature ,BatteryVoltage.   (LSB first)
 *   + Sensor Status Bitmap byte doesn't used any more the defualt value is 0xFA:
 *       * Bit 0 :1 Sensor connected ,0 sensor disconnected.
 *       * Bit 1 :1 Sensor Enabled   ,0 Sensor Disabled.
 *       * Bit 2 :1 Sensor Fail      ,0 Sensor Working Well.
 *   + Battery Voltage(LSB first),The two bytes  is the absolute value multiplied by 2 (to avoid using float number)
 *     so don't forget the divide it by 2 to get the correct measurment.
 *
 * Reference
 * docuement http://www.wireless-iot.beanair.com/files/TN-RF-004-MQTT-Comnmunication-Protocol.pdf (pages: 59, 60)
 * Preparation:
 * 1- Configure the beandevice to use the MQTT mode.
 * 2- Make sure to enable the topic for static measurement of each channel.
 * 3- Start running the beandevice in any mode of acquisition.
 * Steps:
 * 1- connect to the broker.
 * 2- subscribe to diagnostic topic {[MAC_ID]/UPDATE}.
 * 3- wait for payloads.
 * 4- parse the coming payload.
 * 5- display data in the cmd.
 ***********************************************************************************************************************************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MQTTClient.h"

/*
####################################
########### Configuration ##########
####################################
*/

/**@broker config**/
#define  PORT_BROKER     "1883"
#define  ADDRESS_BROKER  "192.168.1.91"                       /*Change the ADDRESS_BROKER with the IP OR HOSTNAME of your broker.*/
#define  CLIENTID        "ExampleClientSubDiagnosticBeanair"   
#define  QOS             1
#define  TIMEOUT         1000L

/**@BeanDevice config**/
#define  DEVICE_MAC_ID    "A4D57843DEDD0000"  

/**@globlal variables**/
#define  TOPIC             DEVICE_MAC_ID "/UPDATE"
#define  ADDRESS           ADDRESS_BROKER ":"  PORT_BROKER
#define  OTAC_LENGTH  50                                      /*The length of buffer that contains the packet.*/
unsigned char   Otac_acq[OTAC_LENGTH];                        /*The buffer that contains the packet.*/

//**@Define type of variables.**/
typedef enum DiagPacket{YEAR=17,MONTH=19,DAY,HOUR,MINUTE,SECOND,DIAG_TYPE,PER=26,LQI=28,DIAG_OPTION=31,INTERNAL_TEMPERATURE=33,
DATA_FREE_MEM=37,ENERGY_H_STATUS,BATTRY_VOLTAGE=42,NUMBER_OF_AVAILABLE_SENSOR=44,SENSOR_DIAGNOSTIC} DiagPacket_t; 
typedef enum StatusBitmap{STATUS_BIT_0=0X01,STATUS_BIT_1=0X02,STATUS_BIT_2=0X04}StatusBitmap_t;
typedef struct s_Date
{
  unsigned long long      m_year;
  unsigned char           m_month;
  unsigned char           m_day;
  unsigned char           m_hour;
  unsigned char           m_minute;
  unsigned char           m_second;
} t_Date;

typedef struct s_DiagnosticPacket
{
  t_Date                   m_dateDignostic;
  unsigned char            m_diagnosticType;
  unsigned long long       m_per ;
  unsigned char            m_lqi;
  unsigned long long       m_diagnosticOptions;
  unsigned long long       m_internalTemperature ;
  unsigned char            m_dataloggerFreeMemory ;
  unsigned char            m_energyHStatus ;
  unsigned long long       m_batteryVoltage ;
  unsigned char            m_numberChannels;
}t_DiagnosticPaket;

/**@Function declaration.**/
void vSlicingPacketDiagnostic(unsigned char *OtacDiagostic);
void vDetectDateDiag(unsigned char *OtacDiagostic,t_Date *s_dateDiag);
void vConcatenateBytes(const unsigned  char * a_Bytes ,unsigned long long *concatenatedBytes ,const unsigned char LENGTH_BUFFER_BYTES );
void vDetectDiagnosticType(const unsigned char diagnosticType,unsigned char *const outDiag);
void vDetectPacketErrorRate(const unsigned char *a_OtacDiagnostic , unsigned long long *p_PacketError );
void vDetectNetworkQuality(const unsigned char InNetworkQuality,unsigned char * const OutNetworkQuality);
void vDetectDiagnosticOption(const unsigned char  *a_OtacDiagostic ,unsigned long long *p_DiagnosticOpt);
void vDetectInternalTemperature(const unsigned char *a_OtacDiagostic,unsigned long long *p_Internal_Temp);
void vDetectLoggerFreeMemory(const unsigned char InLoggerFreeMemory,unsigned char *const OutLoggerFreeMemory);
void vDetectEnergyHarvesterStatus(const unsigned char InEnergyharvesterStatus ,unsigned char *const OutEnergyharvesterStatus );
void vDetectBattryVoltage(unsigned char *a_BattryVoltage ,unsigned long long *p_BattryVoltage );
void vDetectNumberAvailableSensorChannel(const unsigned char  InNumberChannels ,unsigned char *const OutNumberChannels);
void vDetectSensorDiagnostic(const unsigned char  sensorDiagnostic);



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

void vDetectDateDiag(unsigned char *a_OtacDiagostic,t_Date *s_dateDiag)
{
  /**@Detect the date from the diagnostic packet and print it in screen.**/
   vConcatenateBytes(a_OtacDiagostic+YEAR,&(s_dateDiag->m_year) ,2 );
   s_dateDiag->m_month=a_OtacDiagostic[MONTH];
   s_dateDiag->m_day=a_OtacDiagostic[DAY];
   s_dateDiag->m_hour=a_OtacDiagostic[HOUR];
   s_dateDiag->m_minute=a_OtacDiagostic[MINUTE];
   s_dateDiag->m_second=a_OtacDiagostic[SECOND];
   printf("The Date                          =%02u/%02u/%02llu %02d:%02d:%02d",s_dateDiag->m_day,s_dateDiag->m_month,s_dateDiag->m_year,s_dateDiag->m_hour,s_dateDiag->m_minute,s_dateDiag->m_second);
}      

void vDetectDiagnosticType(const unsigned char IndiagnosticType,unsigned char *const outDiagnosticType  ) 
{
  /**@Detect the diagnostic type and print it in the screen**/
  *outDiagnosticType=IndiagnosticType;
  printf("\nThe Diagnostic type               =(0x%02X)Hex   | (%d)Decimal ",*outDiagnosticType ,*outDiagnosticType);
}

void vDetectPacketErrorRate(const unsigned char *a_OtacDiagnostic , unsigned long long *p_PacketError )
{
 /**@Detect the Packet Error Rate from the packet and print it in the screen.**/
  vConcatenateBytes(a_OtacDiagnostic,p_PacketError ,2 );
  printf("\nThe Packet Error Rate             =(0x%02llX)Hex   | (%lld)Decimal ",*p_PacketError,*p_PacketError); 
}

void vDetectDiagnosticOption(const unsigned char  *a_OtacDiagostic ,unsigned long long  *p_DiagnosticOpt)
{
 /**@Detect Diagnostic Option and print it in the screen.**/
  vConcatenateBytes(a_OtacDiagostic,p_DiagnosticOpt ,2 );
  printf("\nThe Diagnostic Option             =(0x%04llX)Hex | (%lld)Decimal ",*p_DiagnosticOpt,*p_DiagnosticOpt);
}

void vDetectNetworkQuality(const unsigned char InNetworkQuality,unsigned char * const OutNetworkQuality)
{
  /**@Detect the Network Quality and print the result in screen.**/
  *OutNetworkQuality=InNetworkQuality;
  printf("\nThe Network Quality               =(0x%02X)Hex   | (%d)Decimal  ",*OutNetworkQuality,*OutNetworkQuality); 
}

void vDetectInternalTemperature(const unsigned char *a_OtacDiagostic,unsigned long long  *p_Internal_Temp)
{
/**@Detect Internal temperature and print it in cmd.**/
 /* NOTE:
  * should divide the obtained value{Internal Temperature} by 2.
  */
  vConcatenateBytes(a_OtacDiagostic,p_Internal_Temp ,2);
  *p_Internal_Temp/=2;
  printf("\nThe Internal Temperature          =(%lld)deg ",*p_Internal_Temp);
} 

void vDetectLoggerFreeMemory(const unsigned char InLoggerFreeMemory,unsigned char *const OutLoggerFreeMemory)
{
  /**@Detect the logger free Memory and print it in cmd.**/
  *OutLoggerFreeMemory=InLoggerFreeMemory;
  printf("\nThe logger Free Memory            =(0x%02X)Hex   | (%d)Decimal ",*OutLoggerFreeMemory,*OutLoggerFreeMemory); 
}

void vDetectEnergyHarvesterStatus(const unsigned char InEnergyharvesterStatus ,unsigned char *const OutEnergyharvesterStatus )
{
  /**@Detect the Energy harvester Status and print data in cmd*/
  *OutEnergyharvesterStatus=InEnergyharvesterStatus;
  printf("\nThe Energy harvester Status       =(0x%02X)Hex   | (%d)Decimal ",*OutEnergyharvesterStatus,*OutEnergyharvesterStatus);
}  

void vDetectBattryVoltage(unsigned char *a_BattryVoltage ,unsigned long long *p_BattryVoltage )
{
/**@Detect Battry Voltage and print data in screen.**/
  vConcatenateBytes(a_BattryVoltage,p_BattryVoltage ,2 );
  float voltage=*p_BattryVoltage;
  voltage=voltage/1000;   //battry voltage in volt
  printf("\nThe Battry Voltage                =(0x%04llX)Hex | (%lld)mVolt  | (%f) volte  ,",*p_BattryVoltage,*p_BattryVoltage,voltage);
}

void vDetectNumberAvailableSensorChannel(const unsigned char  InNumberChannels ,unsigned char *const OutNumberChannels)
{
  /**@Detect Number available sensor channels and print it in the screen.**/
  *OutNumberChannels=InNumberChannels;
  printf("\nThe number of available channels  =(0x%02X)Hex   | (%d)Decimal ",*OutNumberChannels,*OutNumberChannels); 
}

void vDetectSensorDiagnostic(const unsigned char  sensorDiagnostic)
{
  /**@Detect The Sensor Status Bitmap and print it in the screen.**/
  if(sensorDiagnostic & STATUS_BIT_0)
  {
    printf("\nThe Sensor Connected  byte0 =1");
  }
  else
  {
    printf("\nThe Sensor Disonnected byte0 =0");
  }
  if(sensorDiagnostic & STATUS_BIT_1 )
  {
    printf("\nThe Sensor Enabled     byte1 =1");
  }
  else
  {
    printf("\nThe Sensor Diabled     byte1 =0");
  }
  if(sensorDiagnostic & STATUS_BIT_2)
  {
    printf("\nThe Sensor Fail        byte2 =1");
  }
  else
  {
    printf("\nThe Sensor Fail        byte2 =0");
  }
}
void vSlicingPacketDiagnostic(unsigned char *OtacDiagostic)
{
  /**
   * @Receive diagnostic data generated by the device by MQTT protocols 
   * and store the data in the t_DiagnosticPaket data type.
   **/
  t_DiagnosticPaket s_diagnosticPaket;
  /*detect the date :Year/Month/Day Hour:min:sec.*/
  vDetectDateDiag( OtacDiagostic ,&(s_diagnosticPaket.m_dateDignostic));
  /*detect the diagnostic type      */
  vDetectDiagnosticType(OtacDiagostic[DIAG_TYPE],&(s_diagnosticPaket.m_diagnosticType));
  /*detect the Packet Error Rate*/
  vDetectPacketErrorRate(OtacDiagostic+PER,&(s_diagnosticPaket.m_per));
  /*detect the Network Quality*/
  vDetectNetworkQuality(OtacDiagostic[LQI],&(s_diagnosticPaket.m_lqi));
  /*detect the Diagnostic Option*/
  vDetectDiagnosticOption(OtacDiagostic+DIAG_TYPE,&(s_diagnosticPaket.m_diagnosticOptions));
  /*detect the Internal Temperature*/
  vDetectInternalTemperature(OtacDiagostic+INTERNAL_TEMPERATURE,&(s_diagnosticPaket.m_internalTemperature));
  /*detect the logger free Memory*/
  vDetectLoggerFreeMemory(OtacDiagostic[DATA_FREE_MEM],&(s_diagnosticPaket.m_dataloggerFreeMemory)); 
  /*detect the Energy harvester Status*/
  vDetectEnergyHarvesterStatus(OtacDiagostic[ENERGY_H_STATUS],&(s_diagnosticPaket.m_energyHStatus)); 
  /*detect the Battry Voltage*/
  vDetectBattryVoltage(OtacDiagostic+BATTRY_VOLTAGE ,&(s_diagnosticPaket.m_batteryVoltage));
  /*detect Number available sensor channels */
  vDetectNumberAvailableSensorChannel(OtacDiagostic[NUMBER_OF_AVAILABLE_SENSOR] ,&(s_diagnosticPaket.m_numberChannels));
  /*detect sensor diagnostic */
  /* It doesn't used any more ,this configuration is default one{doesn't change in all the case  */
    printf("\n************************************************************************************");
    printf("\n*Note :Sensor Diagnostic doesn't used any more ,This configuration is default one. *");
    printf("\n************************************************************************************");
  for(int channelsIndex=0 ;channelsIndex<(int )s_diagnosticPaket.m_numberChannels;channelsIndex++)
  {
    printf("\n********channel %d ***********",channelsIndex);
    vDetectSensorDiagnostic(OtacDiagostic[SENSOR_DIAGNOSTIC+channelsIndex]);
  }
}

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
    printf("#######################################################\n");
    printf("Message arrived\n");
    printf("     topic: %s\n", topicName);
    payloadptr = message->payload;
    for(i=0; i<message->payloadlen; i++)
    {
        Otac_acq[i]=*payloadptr++;
        /*  Uncomment  this line to see the packet in hex */
        /*printf("0x%02X ,",Otac_acq[i]);*/
    }
    /**@Slicing the data come from the topic.**/
    vSlicingPacketDiagnostic(Otac_acq);
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
    int rc;
    int ch;
    MQTTClient_create(&client, ADDRESS, CLIENTID,MQTTCLIENT_PERSISTENCE_NONE, NULL);
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
