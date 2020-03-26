'''
Company: BeanAir
Date:   24/03/2020
Author: Habib jomaa
Description:
This Example uses MQTT protocol to collect data measurments from a BeanDevice in low duty cycle mode and display it in graph. 
The beandevice used is "X-Inc" with 5 channels active and the broker is "broker.hivemq.com", so make sure to:
    + set your configuration right before running the example.
    + connect this program and the BeanDevice to the same broker.

Informations we need:
    LDCDA (Low Duty Cycle Data Acquisition) frame content:
+(0)---------+(1)------+(2)--------+(3)------------+(7)---------+(10)
|Device type |DAC type |Channel id |Reference time |Data sample |
|(1 byte)    |(1 byte) |(1byte)    |(4 bytes)      |(3 bytes)   |
+------------+---------+-----------+---------------+------------+

(Note: 
    + Date time is in Unix format (LSB first)
    + Data sample (LSB first), the last bit is a sign bit)

Reference
docuement http://www.wireless-iot.beanair.com/files/TN-RF-004-MQTT-Comnmunication-Protocol.pdf (pages: 46, 47)

Preparation:
1- Configure the beandevice to use the MQTT mode
2- Make sure to enable the topic for static measurement of each channel
3- Start running the beandevice in LDC mode

Steps:
1- connect to the broker
2- subscribe to each sensor's static measurement topic
3- wait for payloads
4- check DAC type if it's LDC
5- parse the coming payload
6- display data in graph
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
## broker config
brokerIp = "broker.hivemq.com"
brokerPort = 1883
## beandevice
deviceMacId = "F0B5D1A48F4E0000"
### Note: all topics should be set in the beandevice using the beanscape software (select device -> click BeanDevice -> MQTT)
### The LDC mode uses one topic for each channel, so this list will store the channel name with the corresponding topic
### make sure to subscripe to an existing topic, mean the cahnnel is active. (necessery for the graph)
ldcTopics=[{
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
## graph
### Measurements axeis maximum length
maxSize = 100
### Time axis length
timeAxisRange = 10

####################################
############# Variables ############
####################################
## store the received measurment of each channel in channelsData array, where datas is a table of json ({data and time}) for each measurment)
channelsData = []
for topic in ldcTopics:
    channelsData.append({
        "channelName":topic["channelName"],
        "datas":[]
    })
deviceType = ""
deviceDACmode = ""
## graph
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
        return "Ldc Math Result"
    elif(value==0x06):
        return "S.E.T"
    elif(value==0x07):
        return "Dynamic math result"
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

# return data sample measurment in float format (3 bytes, LSB first and last bit is sign bit)
def get_data(payload):
    data = 0x000000
    data |= payload[0]
    data |= payload[1] << 8
    data |= (payload[2]&0x7f) << 16
    # if sign bit equal 1 the measurement is negative
    if(payload[2]&0x8f==0x80):
        data *= -1
    data /=1000
    return data

# parse the payload message received from beandevice and return all informations
def parsePayloadBeandevice(payload):
    # Device type 1 byte
    deviceType = get_deviceType(payload[0])
    # Device DAC mode in 1 byte
    deviceDACmode = get_deviceDacMode(payload[1])
    # Channel name
    channelName = get_channelName(payload[2])
    # Datetime in 4 bytes
    channelDateTime = get_time(payload[3:7])
    # Get date measurment
    channelData = get_data(payload[7:])
    return deviceType, deviceDACmode, channelName, channelDateTime, channelData


# check if DAC mode of the device is LDC
def isDeviceDacModeLDC(payload):
    deviceDacMode = get_deviceDacMode(payload[1])
    if(deviceDacMode=="LowDutyCycle"):
        return True
    return False 

# This function is called periodically from FuncAnimation
def animate(frameIndex):    
    global channelsData, deviceType, deviceDACmode
    nbrChannel = len(channelsData)
    oldTimeArray = [[] for x in range(nbrChannel)]
    oldDataArray = [[] for x in range(nbrChannel)]
    axs = [0 for x in range(nbrChannel)]
    if(channelsData != []):
        for index, channelData in enumerate(channelsData):
            # data size limit for each channel
            channelData["datas"] = channelData["datas"][maxSize*-1:]
            # create plot for each channel
            axs[index] = fig.add_subplot(nbrChannel, 1, index+1)
            # create axes buffer
            for element in channelData["datas"]:
                oldTimeArray[index].append(element["time"])
                oldDataArray[index].append(element["data"])
        # drow graph for each channel
        for index, channelData in enumerate(channelsData):
            axs[index].clear()
            axs[index].plot(oldTimeArray[index], oldDataArray[index])
            if(index < nbrChannel-1):
                axs[index].get_shared_x_axes().join(axs[index+1], axs[index])
                axs[index].set_xticks([], )
            else:
                if(len(channelData["datas"])<maxSize):
                    slice =int(len(channelData["datas"])/timeAxisRange)+1
                else:
                    slice = int(maxSize / timeAxisRange)+1
                axs[index].set_xticks(axs[index].get_xticks()[::slice])
            # Format plot
            plt.xticks(rotation=30, ha='right')
            plt.subplots_adjust(bottom=0.30)
            axs[index].title.set_text(channelData["channelName"])
    fig.suptitle(deviceType+"  \""+deviceDACmode+"\" ", fontsize=16)
    fig.tight_layout(rect=[0, 0.03, 1, 0.95])

####################################
############# Callbacks ############
####################################
# callback function for the connection with the broker
def on_connect(client, userdata, flags, rc):
    print("Connecting to the MQTT broker, done!")
    for topic in ldcTopics:
        client.subscribe(topic["topic"])
    print("Subscribing to LDC topics, done!")

# callback function for the upcoming messages
def on_message(client, userdata, msg):
    global channelsData, deviceType, deviceDACmode
    # check if the message is a Ldc message
    if(not isDeviceDacModeLDC(msg.payload)):
        return
    deviceType, deviceDACmode, channelName, channelDateTime, channelData = parsePayloadBeandevice(msg.payload)
    # store data
    for element in channelsData:
        if element["channelName"] == channelName:
            element["datas"].append({
                "time": channelDateTime,
                "data": channelData
            })

####################################
########### Main section ###########
####################################
print("Start the MQTT Alarm client")
## MQTT
client = mqttClient.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect(brokerIp, brokerPort)
client.loop_start()
## Graph
ani = animation.FuncAnimation(fig, animate, interval=1000)
plt.show()
