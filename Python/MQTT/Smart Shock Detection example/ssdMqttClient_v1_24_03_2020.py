'''
Company: BeanAir
Date:   24/03/2020
Author: Habib jomaa
Description:
This Example uses MQTT protocol to collect data measurments from a BeanDevice in SSD mode and display it in graph. 
The beandevice used is "X-Inc" with 5 channels active and the broker is "broker.hivemq.com", so make sure to:
    + set your configuration right before running the example.
    + connect this program and the BeanDevice to the same broker.

Informations we need:
    S.E.T frame content:
+(0)---------+(1)------+(2)-------+(8)----------------+(10)------------+(14)--------------+(17)-----------------------+(19)-------------+(22)---------+(25)--------------------------------+(27)-----+(28)-----+(29)---------+(30)--------------+(32)--------------+(34)--------------+(36)-----+(36+n)
|Device type |DAC type |time      |sampling frequency |channels bitmap |frame sequence id |number of data per channel |Monitoring cycle |DAC duration |previouse number of dac per channel |Flags    |LOI      |Shock source |X axis first data |Y axis first data |Z axis first data |Data     |
|(1 byte)    |(1 byte) |(6 bytes) |(2 bytes)          |(4 bytes)       |(3 bytes)         |(2 bytes)                  |(3 bytes)        |(3 bytes)    |(2 bytes)                           |(1 byte) |(1 byte) |(1 byte)     |( 2bytes)         |( 2bytes)         |( 2bytes)         |(n bytes)| 
+------------+---------+----------+-------------------+----------------+------------------+---------------------------+-----------------+-------------+------------------------------------+---------+---------+-------------+------------------+------------------+------------------+---------+

    Time:
+---------------+----------------------+
|Reference time |Reference millisecond | 
|(4 bytes)      |(2 bytes)             |
+---------------+----------------------+

    Flags field:
+--------------- +-----------+
|Synchronization |future use | 
|(1 bit)         |(7 bits)   |
+----------------+-----------+

    Data field:
<-------------------------- sample 1 of each active channel ----------------------------------><--------------- sample 2 of each active channel ------------>       <--------------- sample m of each active channel -------------->
+----------------------------+----------------------------+------+----------------------------+----------------------------+----+----------------------------+-----+-----------------------------+----+-----------------------------+
|Data sample 1 for channel 1 |Data sample 1 for channel 2 | .... |Data sample 1 for channel n |Data sample 2 for channel 1 | .. |Data sample 2 for channel n | ... | Data sample m for channel 1 | .. | Data sample m for channel n |
|(3 bytes)                   |(3 bytes)                   | .... |(3 bytes)                   |(3 bytes)                   | .. |(3 bytes)                   | ... | (3 bytes)                   | .. | (3 bytes)                   |
+----------------------------+----------------------------+------+----------------------------+----------------------------+----+----------------------------+-----+-----------------------------+----+-----------------------------+
(Note:  Data sample in 3 bytes, the last bit is a sign bit and all field are LSB first
        X, Y and Z first data (2 bytes) uses the 2's compelement format)

Reference
docuement http://www.wireless-iot.beanair.com/files/TN-RF-004-MQTT-Comnmunication-Protocol.pdf (pages: 46, 52)

Preparation:
1- Configure the beandevice to use the MQTT mode
2- Make sure to enable the topic for dynamic measurement
3- Start running the beandevice in Shock detection mode

Steps:
1- connect to the broker
2- subscribe to the topic "STREAMING" (it is the same topic for streaming, S.E.T and SSD mode) to receive tx messages
3- wait for payloads
4- check DAC type if it's SSD
5- parse the coming payload
6- check if it's a new streaming using the frame id (frame id = 0) to cleare graph axes
    (the alternative way is to subscribe the the UPDATE topic {see S.E.T example"})
7- display data in graph
'''

####################################
############# Libraries ############
####################################
import paho.mqtt.client as mqttClient  # MQTT package to connect and communicate with the broker
import time
import datetime
from matplotlib import pyplot as plt, animation # Plot package for the graph

####################################
########### Configuration ##########
####################################
# (Edite this section with the configuration you have)
## broker
brokerIp = "broker.hivemq.com"
brokerPort = 1883
## beandevice
deviceMacId = "F0B5D1A48F4E0000"
### Note: all topics should be set in the beandevice using the beanscape software (select device -> click BeanDevice -> MQTT)
### Because we will listen only to the SSD mode DAC we will need only the topic for streaming mode (it's the same topic for the Streaming S.E.T and SSD mode)
ssdTopic = deviceMacId+"/STREAMING"
## graph
### Measurements axeis maximum length
maxSize = 2000
### Time axis length
timeAxisRange = 10

####################################
############# Variables ############
####################################
## Graph
### storage variables for the time and data acquisition of the graph
### size of the array is 5 because the maximum number of channel we can have is 5 (ch_x, ch_y, ch_z, x_inc, y_inc)
oldTimeArray = [[] for x in range(5)]
oldDataArray = [[] for x in range(5)]
### figure
fig = plt.figure()

####################################
############# Functions ############
####################################
# return the device type in string format
def get_deviceType(value):
    if(value==0x01):
        return "AX 3D"
    elif(value==0x02):
        return "HI INC MONO" # this device is no longer exist in the market
    elif(value==0x03):
        return "HI INC BI"
    elif(value==0x04):
        return "X- INC MONO" # this device is no longer exist in the market
    elif(value==0x05):
        return "X- INC BI"
    elif(value==0x06):
        return "AX 3DS"
    else:
        return "unknow"

# return the device DAC mode in string format
def get_deviceDacMode(value):
    if(value==0x01):
        return "LowDutyCycle"
    elif(value==0x02):
        return "Alarm"
    elif(value==0x03):
        return "Streaming"
    elif(value==0x04):
        return "Shock Detection"
    elif(value==0x05):
        return "later"
    elif(value==0x06):
        return "S.E.T"
    else:
        return "unknow"

# return the channel name in string format
def get_channelName(value):
    if(value==0x00):
        return "Ch_Z"
    elif(value==0x01):
        return "Ch_X"
    elif(value==0x02):
        return "Ch_Y"
    elif(value==0x03):
        return "Inc_X"
    elif(value==0x04):
        return "Inc_Y"
    else:
        return "unknow"

# return a list of enabled channels from bitmap field (field in 4 bytes)
# each bit in the bitmap field reserver for one channel to indicate its status (enable or disable)
# E.g: if bit 0 in the bitmap field equal to 1 then the channel 0 is enable, if 0 then channel 0 is disable. (the same for the rest of channels)
def get_activeChannels(payload):
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

# return the frame sequence id which is in 3 byte (LSB first) in int format
def get_frameId(payload):
    frameId = 0x000000
    frameId |= payload[0]
    frameId |= payload[1]<<8
    frameId |= payload[2]<<16
    frameId = int(frameId)
    return frameId

# return the number of data per channel in int format
def get_numberOfDataPerChannel(payload):
    # number of data per channel is in 2 bytes and it's an unsigned number
    nbrData = 0x0000
    nbrData |= payload[0]
    nbrData |= payload[1]<<8
    return int(nbrData)

# return the previous number of data per channel in int format
def get_previousNumverOfDataPerChannel(payload):
    # previous number of data per channel is in 2 bytes and it's an unsigned number
    previousNbrData = 0x0000
    previousNbrData |= payload[0]
    previousNbrData |= payload[1]<<8
    return int(previousNbrData)

# return the syncronization status in sting format
def get_synchronizationStatus(payload):
    # synchronization status is the first bit
    if(payload&0x01 == 1):
        return "synchronized"
    else:
        return "not synchronized"

# return time in array [timestamp(int), milliseconds (int)]
def get_time(payload):
    payload_s = payload[0:4]
    ## get date and time 
    timestamp = 0x00000000
    timestamp |= payload_s[0]
    timestamp |= payload_s[1]<<8
    timestamp |= payload_s[2]<<16
    timestamp |= payload_s[3]<<24
    timestamp = int(timestamp)
    ## get milliseconds
    payload_ms = payload[4:6]
    millisecond = 0x0000
    millisecond |= payload_ms[0]
    millisecond |= payload_ms[1]<<8
    millisecond = int(millisecond)
    return timestamp, millisecond

#  return the sampling frequency which is in 2 bytes (LSB first)
def get_samplingRate(payload):
    samplingRate = 0x0000
    samplingRate |= payload[0]
    samplingRate |= payload[1]<<8
    return int(samplingRate)

# return array of each sub packet date and time (the sub packet is the number of data per channel)
def get_dateTimeArray(frameId, timestamp, millisecond, samplingRate, nbrDataPerChannel, previousNbrDataPerChannal):
    dateTimeArray = [0 for i in range(nbrDataPerChannel)]
    for subPacketRow in range(nbrDataPerChannel):
        subPacketIndex = (frameId * previousNbrDataPerChannal) + subPacketRow
        dateTimeArray[subPacketRow] = timestamp + millisecond/1000 + ((1/samplingRate) * subPacketIndex)
    return dateTimeArray

# calculate data
# return measurement value from data in 2 bytes (X, Y and Z first data)
# data in 2's complement
def firstDataCalculator(payload):
    value = int.from_bytes(payload, byteorder="little", signed=True)
    value = value/1000
    return value

# return measurement value from data in 3 bytes
# data is in LSB first foramt, the last bit is a sign bit and the rest is the absolute value
def dataCalculator(payload):
    value = 0x000000
    value |= payload[0]
    value |= (payload[1]<<8)
    value |= (payload[2]&0x7f<<16)
    data = int(value)/1000
    #if sign bit equal 1 the measurement is negative
    if((payload[2]&0x80)==0x80):
        data *= -1
    return data

# get_data function will return n channel and each channel has its data along with the matched time
'''
+--------------+-------+-------+
| channel name |     data      |
+--------------+-------+-------+
|     Ch_Z     |  time | value |
|              |-------+-------|
|              |  T1   |  D1   |
|              +-------+-------+
|              |  time | value |
|              |-------+-------|
|              |  T2   |  D3   |
|              +-------+-------+
|              |       .       |
|              |       .       |
|              |       .       |
|              +-------+-------+
|              |  time | value |
|              |-------+-------|
|              |  Tn   |  Dn   |
+--------------+-------+-------+
               .
               .
               .
+--------------+-------+-------+
|     X_Inc    |  time | value |
|              |-------+-------|
|              |  T1   |  D1   |
|              +-------+-------+
|              |  time | value |
|              |-------+-------|
|              |  T2   |  D3   |
|              +-------+-------+
|              |       .       |
|              |       .       |
|              |       .       |
|              +-------+-------+
|              |  time | value |
|              |-------+-------|
|              |  Tn   |  Dn   |
+--------------+-------+-------+
the way to create this nested array is to:
 1- create array of 2 dimensions {first element is the channel name, the second element is the data}
 2- the second element, which is the data of that channel, is an array of n dimention, where n is the number of data per channel
 3- each element in the data section is a 2 dimensions array for time and value (data acquisiton values)

 Note: you are free to use any storage architecture (like JSON format, or array with different dimension strategy)
'''
def get_data(payload, channels, nbrDataPerChannel, dateTimeArray):
    nbrChannel = len(channels)
    channelsData = [["",[[0,0] for x in range(nbrDataPerChannel)]] for y in range(nbrChannel)]
    for i in range(nbrChannel):
        channelsData[i][0] = channels[i]
        for j in range(nbrDataPerChannel):
            # the position of the first byte of the data for a channel can be calculated in this way:
            # because the size of the measurement(we will call it sample) is 3 byte and it's grouped by sample which mean each sub packet of the whole data samples represent one sample for each channel 
            # (see Table 25: STREAMING frame contents seen from data consumer side in the PDF http://www.wireless-iot.beanair.com/files/TN-RF-004-MQTT-Comnmunication-Protocol.pdf)
            # the next data will be in the next sub packet wich start from "the number of channel * size of one sample * current position"
            pos = i*3+j*nbrChannel*3
            # save data
            channelsData[i][1][j][0]=datetime.datetime.utcfromtimestamp(dateTimeArray[j]).strftime('%Y-%m-%d %H:%M:%S.%f')
            channelsData[i][1][j][1]=dataCalculator(payload[pos:pos+3])
    return channelsData

# parse the payload message received from beandevice and return all informations in string format
def parsePayloadBeandevice(payload):
    # Device type 1 byte
    deviceType = get_deviceType(payload[0])
    # Device DAC mode in 1 byte
    deviceDACmode = get_deviceDacMode(payload[1])
    # Datetime in 6 bytes
    deviceDateTime, deviceTimeMillisecond = get_time(payload[2:2+6])
    # Sampling rate in 2 bytes
    deviceSamplingRate = get_samplingRate(payload[8:8+2])
    # Channels bitmap in 4 bytes
    channels = get_activeChannels(payload[10:10+4])
    # Frame sequence id in 3 bytes
    frameId = get_frameId(payload[14:14+3])
    # Number of data acquisitions per channel in 2 bytes
    nbrSamplePerChannel = get_numberOfDataPerChannel(payload[17:17+2])
    # Previouse Number of data acquisition per channel in 2 bytes
    previousNbrSamplePerChannel = get_previousNumverOfDataPerChannel(payload[25:25+2])
    # Shock source in 1 byte
    print("shock source")
    print(get_channelName(payload[29]))
    # X, Y and Z axis first data
    print("X = %f"%firstDataCalculator(payload[30:32]))
    print("Y = %f"%firstDataCalculator(payload[32:34]))
    print("Z = %f"%firstDataCalculator(payload[34:36]))
    # Get the synchronization status
    synchronizationStatus = get_synchronizationStatus(payload[27])
    # Get the list of dates and times (time in millisecond)
    dateTimeArray = get_dateTimeArray(frameId, deviceDateTime, deviceTimeMillisecond, deviceSamplingRate, nbrSamplePerChannel, previousNbrSamplePerChannel)
    # Get the list of data acquisition for each channel with its corresponding time
    channelsData = get_data(payload[36:], channels, nbrSamplePerChannel, dateTimeArray)
    return deviceType, deviceDACmode, synchronizationStatus, channelsData, frameId


# check if DAC mode of the device is streaming
def isDeviceDacModeSet(payload):
    deviceDacMode = get_deviceDacMode(payload[1])
    if(deviceDacMode=="Shock Detection"):
        return True
    return False 

# graph function to display measurement
def graph(channelsData, deviceType, synchronizationStatus, deviceDACmode, xData, yData):
    if(channelsData != []):
        nbrChannel = len(channelsData)
        axs = [0 for x in range(5)]
        fig.clf()
        fig.clear()
        for index, channelData in enumerate(channelsData):
            axs[index] = fig.add_subplot(nbrChannel, 1, index+1)
            axs[index].clear()
            axs[index].plot(xData[index], yData[index])
        for index, channelData in enumerate(channelsData):
            # Draw x and y lists
            if(index < nbrChannel-1):
                axs[index].get_shared_x_axes().join(axs[index+1], axs[index])
                axs[index].set_xticks([], )
            else:
                if(len(xData[index])<maxSize):
                    slice =int(len(xData[index])/timeAxisRange)+1
                else:
                    slice = int(maxSize / timeAxisRange)+1
                axs[index].set_xticks(axs[index].get_xticks()[::slice])
            axs[index].title.set_text(channelData[0])
        # Format plot
        plt.xticks(rotation=45, ha='right')
        plt.subplots_adjust(bottom=0.30)
        fig.suptitle(deviceType+"  \""+deviceDACmode+"\" "+"(" + synchronizationStatus +")", fontsize=16)
        fig.tight_layout(rect=[0, 0.03, 1, 0.95])
        plt.pause(0.05)

####################################
############# Callbacks ############
####################################
# callback function for the connection with the broker
def on_connect(client, userdata, flags, rc):
    print("Connection to the MQTT broker, done!")
    client.subscribe(ssdTopic)
    print("Subscription to STREAMING topic, done!")

# callback function for the upcoming messages
def on_message(client, userdata, msg):
    global oldTimeArray, oldDataArray
    # check if the payload of the message come from a S.E.T DAC mode
    if(isDeviceDacModeSet(msg.payload)):
        deviceType, deviceDACmode, synchronizationStatus, channelsData, frameId = parsePayloadBeandevice(msg.payload)
        # if the frame id is 0, it means it's a new streaming so cleare time and data axes for new graph
        if(frameId == 0):
            oldTimeArray = [[] for x in range(5)]
            oldDataArray = [[] for x in range(5)]
        # prepare and store data for the graph axes
        # loop through each channel in channelsData
        for index, channelData in enumerate(channelsData):
            #loop through each element in channelData (an element is an array of 2 dimensions for {time and value})
            for data in channelData[1]:
                oldTimeArray[index].append(data[0])
                oldDataArray[index].append(data[1])
            # limit the data of each axis to not pass the max size
            oldTimeArray[index] = oldTimeArray[index][maxSize*-1:]
            oldDataArray[index] = oldDataArray[index][maxSize*-1:]
        # display graph
        graph(channelsData, deviceType, synchronizationStatus, deviceDACmode, oldTimeArray, oldDataArray)

####################################
########### Main section ###########
####################################
print("Start the MQTT SSD client")
## MQTT
client = mqttClient.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect(brokerIp, brokerPort)
client.loop_start()
## Graph
plt.show()
