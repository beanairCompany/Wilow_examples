'''
Company: BeanAir
Date:   17/03/2020
Author: Habib jomaa
Description:
This Example uses MQTT protocol to receive, parse and display the dynamic math result. 
The beandevice used is "X-Inc" with 5 channels active and the broker is "broker.hivemq.com", so make sure to:
    + set your configuration right before running the example.
    + connect this program and the BeanDevice to the same broker.

Informations we need:
    Dynamic Math Result frame content:
+(0)---------+(1)--------------+(2)--------------+(3)-------------------+(5)---------------+(8)------------------+(11)---------------------+(12)------------+(16)------+(25)------+(34)-----------------------+(36)-------------+(36+n)
|Device type |Acquisition type |Acquisition mode |Acquisition frequency |Acquisition cycle |Acquisition duration |Future use(default 0x55) |Channels bitmap |Start time|End time  |Number of data per channel |Math result Data |
|(1 byte)    |(1 byte)         |(1 byte)         |(2 byte)              |(3 bytes)         |(3 bytes)            |(1 bytes)                |(4 bytes)       |(9 bytes) |(9 bytes) |(2 bytes)                  |(n bytes)        |
+------------+-----------------+-----------------+----------------------+------------------+---------------------+-------------------------+----------------+----------+----------+---------------------------+-----------------+

Math result Data field:
+-------------------------+-------------------------+------+-------------------------+
|Math result of channel 1 |Math result of channel 2 | .... |Math result of channel n |
|(21 bytes)               |(21 bytes)               | .... |(21 bytes)               |
+-------------------------+-------------------------+------+-------------------------+

Math result of one channel field:
+---------------+---------------+--------------+
|Minimum result |Maximum result |Average value |
|(9 bytes)      |(9 bytes)      |(3 bytes)     |
+---------------+---------------+--------------+

Minimum/Maximum result field:
+------------------+----------------------------------+------------------+
|Frame Sequence Id |Index of measurement in the frame |Measurement value |
|(4 bytes)         |(2 bytes)                         |(3 bytes)         |
+------------------+----------------------------------+------------------+

(Note:
    + Acquisition mode is Streaming or SSD or S.E.T without the mode option (streaming continue, burst, one shot)
    + Time (Start and End) using this format: Years(first 2 bytes, LSB first) : MMonthsM(1 byte) : Days(1 byte) : Hours(1 byte) : Minutes(1 byte) : Seconds(1 byte) : Milliseconds(last 2 bytes, LSB first)
    + Values (minimum,maximum and average) (LSB first), the last bit is a sign bit)
    + Frame sequence id is the Id of the streaming frame
    + Index of measurement in the frame is the index of the subpacket (see streaming section) in the streaming frame

    
Reference
docuement http://www.wireless-iot.beanair.com/files/TN-RF-004-MQTT-Comnmunication-Protocol.pdf (pages: 46, 55)

Preparation:
1- Configure the beandevice to use the MQTT mode
2- Make sure to enable the topic of the dynamic measurements
3- Start running the beandevice in Streaming burst or streaming one shot or S.E.T or Shock Detection

Steps:
1- connect to the broker
2- subscribe to dynamic math result topic which is the same dynamic measurement topic in the beanscape mqtt configuration of the beandevice(topic: "[MAC_ID]/STREAMING")
3- wait for payloads
4- check DAC type if it's a dynamic Math Result
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
### To receive the Dynamic math result we need to subscribe to the dynamic measurement topic
dynamicMathResultTopic= deviceMacId+"/STREAMING"

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

# get the frame type (works for both, acquisition type and acquisition mode)
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

# return time in this format (Year/Month/Day/ Hours:Minutes:Seconds.Milliseconds) , parameter in 9 bytes
def get_time(payload):
    time = ""
    ## Years (first 2 bytes, LSB first)
    years = 0x0000
    years |= payload[0]
    years |= payload[1]<<8
    years = int(years)
    time += str(years) + "-"
    ## Months (1 byte)
    months = int(payload[2])
    time += str(months) + "-"
    ## Day (1 byte)
    Day = int(payload[3])
    time += str(Day) + " "
    ## Hours (1 byte)
    Hours = int(payload[4])
    time += str(Hours) + ":"
    ## Minutes (1 byte)
    Minutes = int(payload[5])
    time += str(Minutes) + ":"
    ## Seconds (1 byte)
    Seconds = int(payload[6])
    ## milliseconds (2 bytes, LSB first)
    milliseconds = 0x0000
    milliseconds |= payload[7]
    milliseconds |= payload[8]<<8
    milliseconds = int(milliseconds)/1000 + Seconds
    time += str(milliseconds)
    return time

# return acquisition frequency which is in 2 bytes (LSB first)
def get_acquisitionFrequency(payload):
    acquisitionFrequency = 0x0000
    acquisitionFrequency |= payload[0]
    acquisitionFrequency |= payload[1]<<8
    return int(acquisitionFrequency)

# return the number of data per channel which is in 2 bytes (LSB first)
def get_numberOfDataPerChannel(payload):
    numberOfDataPerChannel = 0x0000
    numberOfDataPerChannel |= payload[0]
    numberOfDataPerChannel |= payload[1]<<8
    return int(numberOfDataPerChannel)

# get channels from bitmap field (field in 4 bytes)
# each bit in the bitmap field reserver for one channel to indicate its status (enable or disable)
# E.g: if bit 0 in the bitmap field equal to 1 then the channel 0 is enable, if 0 then channel 0 is disable. (the same for the rest of channels)
def get_channels(payload):
    channelsBitMap = 0
    channelsBitMap |= payload[0]
    channelsBitMap |= payload[1]<<8
    pos = 0x01
    posIndex=0
    channels=[]
    while(pos<=0x80):
        channelValue = channelsBitMap & pos
        if(channelValue&pos):
            channels.append(get_channelName(posIndex))
        pos<<=1
        posIndex+=1
    return channels

# return the frame sequence id which is in 4 bytes (LSB first)
def get_frameId(payload):
    frameId = 0x000000
    frameId |= payload[0]
    frameId |= payload[1]<<8
    frameId |= payload[2]<<16
    frameId |= payload[3]<<24
    frameId = int(frameId)
    return frameId

# return the frame sequence id which is in 2 bytes (LSB first)
def get_index(payload):
    index = 0x000000
    index |= payload[0]
    index |= payload[1]<<8
    index = int(index)
    return index

# return data sample (3 bytes, LSB first and last bit is sign bit) in decimal format
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

# return the event time in string format
def get_eventTime(startTime, frameId, index, nbrDataPerChannel, acquisitionFrequency):
    # convert time from string to timestamp
    startTime_timestamp = datetime.datetime.timestamp(datetime.datetime.strptime(startTime, "%Y-%m-%d %H:%M:%S.%f"))
    # calculate the full index (Note: frame id start from 0 and index represent the data index only in the frame itself)
    fullIndex = frameId * nbrDataPerChannel + index
    # calculate the time of the event
    eventTime_timestamp = startTime_timestamp + 1/acquisitionFrequency * fullIndex
    eventTime = datetime.datetime.fromtimestamp(eventTime_timestamp).strftime('%Y-%m-%d %H:%M:%S.%f')
    
    return eventTime

# return the math result data array of each channel (type: array of json)
'''
[{
    "channelName" : "first channel name",
    "Minimum":{
        "data" : "minimum value",
        "time" : "time of the minimum value"
        },
    "Maximum": {
        "data" : "maximum value",
        "time" : "time of the maximum value"
        },
    "Average": {
        "data" :  "average value"
    }
},
.
.
.
{
    "channelName" : "last channel name",
    "Minimum":{
        "data" : "minimum value",
        "time" : "time of the minimum value"
        },
    "Maximum": {
        "data" : "maximum value",
        "time" : "time of the maximum value"
        },
    "Average": {
        "data" :  "average value"
    }
}]
'''
def get_mathResultData(startTime, nbrDataPerChannel, acquisitionFrequency, channels, payload):
    # frame sequence id in 4 bytes and index in 2 bytes and data in 3 bytes, in total 9 bytes
    frameId_len = 4
    index_len = 2
    data_len = 3
    # each channel has a minimum, a maximum and an average in this order
    mathResultTypes=["Minimum", "Maximum", "Average"]
    pos_start = 0
    channelsMathResult=[]
    for channelName in channels:
        ChannelMathResult = {
            "channelName" : channelName,
            "Minimum":{
                "data" : None,
                "time" : None
            },
            "Maximum":{
                "data" : None,
                "time" : None
            },
            "Average":{
                "data" : None
            }
        }
        for mathResultType in mathResultTypes:
            # get event time of the minimum and the maximum data
            if(mathResultType != "Average"):
                # get frame id (4 bytes)
                pos_stop = pos_start + frameId_len
                frameId = get_frameId(payload[pos_start:pos_stop])
                # get index (2 bytes)
                pos_start = pos_stop
                pos_stop = pos_start + index_len
                index = get_index(payload[pos_start:pos_stop])
                # get event time
                eventTime = get_eventTime(startTime, frameId, index, nbrDataPerChannel, acquisitionFrequency)
                pos_start = pos_stop
                ChannelMathResult[mathResultType]["time"] = eventTime
            # get data (3 bytes)
            pos_stop = pos_start + data_len
            data = get_data(payload[pos_start:pos_stop])
            pos_start = pos_stop
            ChannelMathResult[mathResultType]["data"] = data
            # append the result to the array
        channelsMathResult.append(ChannelMathResult)
    return channelsMathResult

# parse the payload message received from beandevice and return:
# device type (string), acquisition mode (string), start time (string), end time (string), math result array of each channel (array of json)
def parsePayloadBeandevice(payload):
    # Device type 1 byte
    deviceType = get_deviceType(payload[0])
    # Device DAC mode in 1 byte
    deviceDACmode = get_frameType(payload[2])
    # get the acquistion frequency
    acquisitionFrequency = get_acquisitionFrequency(payload[3:5])
    # get the list of active channel
    channels = get_channels(payload[12:16])
    # get start time
    startTime = get_time(payload[16:25])
    # get end time
    endTime = get_time(payload[25:34])
    # get the number of data per channel
    nbrDataPerChannel = get_numberOfDataPerChannel(payload[34:36])
    # get math result array
    mathResultArray = get_mathResultData(startTime, nbrDataPerChannel, acquisitionFrequency, channels, payload[36:])
    return deviceType, deviceDACmode, startTime, endTime, mathResultArray

# check if it's a dynamic math result frame
def isDynamicMathResultFrame(payload):
    frametype = get_frameType(payload[1])
    if(frametype=="Dynamic math result"):
        return True
    return False 

# print function
def printResult(deviceType, startTime, endTime, mathResultArray):
    print("*********************** Dynamic Math Result of " + deviceMacId + " ***************************")
    print("Device: " + deviceType)
    print("start time: " + startTime + " - end time: " + endTime)
    for ChannelMathResult in mathResultArray:
        print("   --- Channel Name: " + ChannelMathResult["channelName"] + " ---   ")
        print("Minimum: %.3f | time: %s" %(ChannelMathResult["Minimum"]["data"], ChannelMathResult["Minimum"]["time"]))
        print("Maximum: %.3f | time: %s" %(ChannelMathResult["Maximum"]["data"], ChannelMathResult["Maximum"]["time"]))
        print("Average data: %.3f" %ChannelMathResult["Average"]["data"])
        print("")
    print("***************************************************************************************")

# callback function for the connection with the broker
def on_connect(client, userdata, flags, rc):
    print("Connecting to the MQTT broker, done!")
    client.subscribe(dynamicMathResultTopic)
    print("Subscribing to Dynamic Math Reasult topic, done!")

# callback function for the upcoming messages
def on_message(client, userdata, msg):
    # check if the message is a dynamic math result message
    if(not isDynamicMathResultFrame(msg.payload)):
        return
    deviceType, deviceDACmode, startTime, endTime, mathResultArray = parsePayloadBeandevice(msg.payload)
    printResult(deviceType, startTime, endTime, mathResultArray)

# main section
## MQTT
client = mqttClient.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect(brokerIp, brokerPort)
client.loop_forever()
