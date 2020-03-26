'''
Company: BeanAir
Date:   11/03/2020
Author: Habib jomaa
Description:
This Example uses MQTT protocol to receive, parse and display the LDC math result.
The beandevice used is "X-Inc" with 5 channels active and the broker is "broker.hivemq.com", so make sure to:
    + set your configuration right before running the example.
    + connect this program and the BeanDevice to the same broker.

Informations we need:
    LDC (Low Duty Cycle) Math Result frame content:
+(0)---------+(1)------+(2)--------+(3)------------+(4)--------+(8)--------+(12)------+(16)--------+(19)
|Device type |DAC type |Channel id |Math result Id |Event time |Start time |End time  |Data sample |
|(1 byte)    |(1 byte) |(1byte)    |(1 byte)       |(4 bytes)  |(4 bytes)  |(4 bytes) |(3 bytes)   |
+------------+---------+-----------+---------------+-----------+-----------+----------+------------+

(Note: 
    + Time (Event, Start and End) is in Unix format (LSB first)
    + Data sample (LSB first), the last bit is a sign bit)
    + The Event time is for the maximum and minimum math result only
    + The Start and End time is for the average math result only

Reference
docuement http://www.wireless-iot.beanair.com/files/TN-RF-004-MQTT-Comnmunication-Protocol.pdf (pages: 46, 54)

Preparation:
1- Configure the beandevice to use the MQTT mode
2- Make sure to enable the topic for static measurement of each channel
3- Start running the beandevice in LDC mode

Steps:
1- connect to the broker
2- subscribe to Ldc math result topics which is the list of static measurement topics in the beanscape MQTT configuration of the beandevice {[MAC_ID]/Sensor0, [MAC_ID]/Sensor1 ... etc}
3- wait for payloads
4- check DAC type if it's LDC Math Result
5- parse the coming payload
6- print the information
'''

# Libraries
import paho.mqtt.client as mqttClient  # MQTT package to connect and communicate with the broker
import time
import datetime
from matplotlib import pyplot as plt, animation # Plot package for the graph

# configurations
## broker config
brokerIp = "broker.hivemq.com"
brokerPort = 1883
## beandevice
deviceMacId = "F0B5D1A48F4E0000"
### Note: all topics should be set in the beandevice using the beanscape software (select device -> click BeanDevice -> MQTT)
### To receive the LDC math result we need to subscribe to the static measurement topics for each channel, so this list will store the channel name with the corresponding topic
ldcMathResultTopics=[{
    "channelName":"Ch_Z",
    "topic":deviceMacId+"/SENSOR/0"
    },
    {
    "channelName":"Ch_X",
    "topic":deviceMacId+"/SENSOR/1"
    },
    {
    "channelName":"Ch_Y",
    "topic":deviceMacId+"/SENSOR/2"
    },
    {
    "channelName":"Inc_X",
    "topic":deviceMacId+"/SENSOR/3"
    },
    {
    "channelName":"Inc_Y",
    "topic":deviceMacId+"/SENSOR/4"
    }]

# get the device type
def get_deviceType(payload):
    if(payload==0x01):
        return "AX 3D"
    elif(payload==0x02):
        return "HI INC MONO" # this device is no longer exist in the market
    elif(payload==0x03):
        return "HI INC BI"
    elif(payload==0x04):
        return "X- INC MONO" # this device is no longer exist in the market
    elif(payload==0x05):
        return "X- INC BI"
    elif(payload==0x06):
        return "AX 3DS"
    else:
        return "unknow"

# get the DAC mode
def get_frameType(payload):
    if(payload==0x01):
        return "LowDutyCycle"
    elif(payload==0x02):
        return "Alarm"
    elif(payload==0x03):
        return "Streaming"
    elif(payload==0x04):
        return "Shock Detection"
    elif(payload==0x05):
        return "Ldc Math Result"
    elif(payload==0x06):
        return "S.E.T"
    elif(payload==0x07):
        return "Dynamic math result"
    else:
        return "unknow"

#get the channel name from channel id (1 byte)
def get_channelName(payload):
    if(payload==0x00):
        return "Ch_Z"
    elif(payload==0x01):
        return "Ch_X"
    elif(payload==0x02):
        return "Ch_Y"
    elif(payload==0x03):
        return "Inc_X"
    elif(payload==0x04):
        return "Inc_Y"
    else:
        return "unknow"

# get the math result type
def get_mathResultType(payload):
    if(payload==0x01):
        return "Maximum"
    elif(payload==0x02):
        return "Minimum"
    elif(payload==0x03):
        return "Average"
    else:
        return "unknow"

# get time (4 bytes, unix format)
def get_time(payload):
    ## get date time 
    timestamp = 0x00000000
    timestamp |= payload[0]
    timestamp |= payload[1]<<8
    timestamp |= payload[2]<<16
    timestamp |= payload[3]<<24
    timestamp = int(timestamp)
    return datetime.datetime.fromtimestamp(timestamp).strftime('%Y-%m-%d %H:%M:%S')

# get data sample (3 bytes, LSB first and last bit is sign bit)
def get_data(payload):
    data = 0x000000
    data |= payload[0]
    data |= payload[1] << 8
    data |= (payload[2]&0x7f) << 16
    # if sign bit equal 1 the data is negative
    if(payload[2]&0x8f==0x80):
        data *= -1
    data /=1000
    return data

# parse the payload message received from beandevice and return all informations in string format
def parsePayloadBeandevice(payload):
    # Device type 1 byte
    deviceType = get_deviceType(payload[0])
    # Device DAC mode in 1 byte
    deviceDACmode = get_frameType(payload[1])
    # Channel name in 1 byte
    channelName = get_channelName(payload[2])
    # math result type in 1 byte
    mathResultType = get_mathResultType(payload[3])
    # if the math result type is average then get the start and the end time, otherwise get the event time
    eventTime = None
    startTime = None
    endTime = None
    if(mathResultType == "Average"):
        startTime = get_time(payload[8:12])
        endTime = get_time(payload[12:16])
    else:
        eventTime = get_time(payload[4:8])
    # Get date measurment
    mathResultData = get_data(payload[16:19])
    return deviceType, deviceDACmode, channelName, mathResultType, eventTime, startTime, endTime, mathResultData

# check if it's a math result frame
def isLdcMathResultFrame(payload):
    frametype = get_frameType(payload[1])
    if(frametype=="Ldc Math Result"):
        return True
    return False 

# print function
def printResult():
    print("*********************** LDC Math Result of " + deviceMacId + " ***************************")
    print("Device: " + deviceType)
    for ChannelMathResult in ChannelsMathResult:
        if( ChannelMathResult["mathResultData_max"] != None or ChannelMathResult["mathResultData_min"] != None or ChannelMathResult["mathResultData_avg"] != None):
            print("Channel Name: " + ChannelMathResult["channelName"])
            print("Average data: " + str(ChannelMathResult["mathResultData_avg"]) + "| start time: " + ChannelMathResult["startTime"] + " - end time: " + ChannelMathResult["endTime"])
            print("Maximum: " + str(ChannelMathResult["mathResultData_max"]) + "| time: " + ChannelMathResult["eventTime_max"])
            print("Minimum: " + str(ChannelMathResult["mathResultData_min"]) + "| time: " + ChannelMathResult["eventTime_min"])
    print("***************************************************************************************")

# callback function for the connection with the broker
def on_connect(client, userdata, flags, rc):
    print("Connecting to the MQTT broker, done!")
    for topic in ldcMathResultTopics:
        client.subscribe(topic["topic"])
    print("Subscribing to LDC topics, done!")

# callback function for the upcoming messages
def on_message(client, userdata, msg):
    global deviceType, deviceDACmode, ChannelsMathResult
    # check if the message is a Ldc math result message
    if(not isLdcMathResultFrame(msg.payload)):
        return
    deviceType, deviceDACmode, channelName, mathResultType, eventTime_tmp, startTime_tmp, endTime_tmp, mathResultData = parsePayloadBeandevice(msg.payload)
    for ChannelMathResult in ChannelsMathResult:
        if(ChannelMathResult["channelName"] == channelName):
            if(mathResultType == "Maximum"):
                ChannelMathResult["eventTime_max"] = eventTime_tmp
                ChannelMathResult["mathResultData_max"] = mathResultData
            elif(mathResultType == "Minimum"):
                ChannelMathResult["eventTime_min"] = eventTime_tmp
                ChannelMathResult["mathResultData_min"] = mathResultData
            elif(mathResultType == "Average"):
                ChannelMathResult["startTime"] = startTime_tmp
                ChannelMathResult["endTime"] = endTime_tmp
                ChannelMathResult["mathResultData_avg"] = mathResultData
            else:
                return()
            break
    printResult()

# main section
## global variabls
deviceType = None
deviceDACmode = None
ChannelsMathResult = [{
    "channelName" : ldcMathResultTopics[x]["channelName"],
    "eventTime_max" : None,
    "eventTime_min" : None,
    "startTime" : None,
    "endTime" : None,
    "mathResultData_max" : None,
    "mathResultData_min" : None,
    "mathResultData_avg" : None
    } for x in range(len(ldcMathResultTopics))]
## MQTT
client = mqttClient.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect(brokerIp, brokerPort)
client.loop_forever()
