'''
Company: BeanAir
Date:   24/03/2020
Author: Habib jomaa
Description:
This Example uses MQTT protocol to collect data measurments from a BeanDevice in Alarm mode and display it in graph along
with the alarm status for each channel. 
The beandevice used is "X-Inc" with 5 channels active and the broker is "broker.hivemq.com", so make sure to:
    + set your configuration right before running the example.
    + connect this program and the BeanDevice to the same broker.

Informations we need:
    Alarm frame content:
+(0)---------+(1)------+(2)--------+(3)------------+(7)----------+(8)---------+(11)
|Device type |DAC type |Channel id |Reference time |Alarm status |Data sample |
|(1 byte)    |(1 byte) |(1byte)    |(4 bytes)      |(1 byte)     |(3 bytes)   |
+------------+---------+-----------+---------------+-------------+------------+

(Note: 
    + Date time is in Unix format (LSB first)
    + Alarm status are 4:
        * 0x00 No Alarm
        * 0x01 Alarm start
        * 0x02 Alarm in progress
        * 0x03 Alarm end
    + Data sample (LSB first), the last bit is a sign bit and the rest is the absolute value multiplied by 1000 (to avoid using float number)
      so don't forget the divide it by 1000 to get the correct measurment

Reference
docuement http://www.wireless-iot.beanair.com/files/TN-RF-004-MQTT-Comnmunication-Protocol.pdf (pages: 46, 47)

Preparation:
1- Configure the beandevice to use the MQTT mode
2- Make sure to enable the topic for static measurement of each channel
3- Don't forget to set the threshold of each channel
4- Start running the beandevice in Alarm mode

Steps:
1- connect to the broker
2- subscribe to each sensor's topic (static measurment topics)
3- wait for payloads
4- check DAC type if it's Alarm
5- parse the coming payload
6- display data in graph
7- display the Alarm status for each channel
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
### The static measurment create one topic for each channel, so this list will store the topic with the corresponding channel name
### make sure to subscripe to an existing topic, mean the cahnnel is active. (necessery for the graph)
alarmTopics=[{
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
## store the received measurment of each channel along with the alarm status in channelsData array, where datas is a table of json ({data and time}) for each measurment)
channelsData = []
for topic in alarmTopics:
    channelsData.append({
        "channelName":topic["channelName"],
        "alarmStatus":"",
        "datas":[]
    })
deviceType = ""
deviceDACmode = ""
## Graph
### figure
fig = plt.figure()

####################################
############# Functions ############
####################################
# return the device type in string format
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

# return the device DAC mode in string format
def get_deviceDacMode(payload):
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

#  return the channel name in string format 
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

# return time in string format (4 bytes, unix format)
def get_time(payload):
    ## get date time 
    timestamp = 0x00000000
    timestamp |= payload[0]
    timestamp |= payload[1]<<8
    timestamp |= payload[2]<<16
    timestamp |= payload[3]<<24
    timestamp = int(timestamp)
    return datetime.datetime.fromtimestamp(timestamp).strftime('%Y-%m-%d %H:%M:%S')

# return alarm status in string foramt (1 byte)
def get_alarmStatus(payload):
    if(payload==0x00):
        return "No alarm"
    elif(payload==0x01):
        return "Alarm Start"
    elif(payload==0x02):
        return "Alarm in progress"
    elif(payload==0x03):
        return "Alarm End"
    else:
        return "unknow"

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
    # Alarm in 1 byte
    alarmStatus = get_alarmStatus(payload[7])
    # Get date measurment
    channelData = get_data(payload[8:])
    return deviceType, deviceDACmode, channelName, channelDateTime, alarmStatus, channelData


# check if DAC mode of the device is Alarm
def isDeviceDacModeAlarm(payload):
    deviceDacMode = get_deviceDacMode(payload[1])
    if(deviceDacMode=="Alarm"):
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
            plt.xticks(rotation=45, ha='right')
            plt.subplots_adjust(bottom=0.30)
            axs[index].title.set_text(channelData["channelName"] + "          ["+ channelData["alarmStatus"] +"]")
    fig.autofmt_xdate()
    fig.suptitle(deviceType+"  \""+deviceDACmode+"\" ", fontsize=16)
    fig.tight_layout(rect=[0, 0.03, 1, 0.95])
    
####################################
############# Callbacks ############
####################################
# callback function for the connection with the broker
def on_connect(client, userdata, flags, rc):
    print("Connecting to the MQTT broker, done!")
    for topic in alarmTopics:
        client.subscribe(topic["topic"])
    print("Subscribing to Alarm topics, done!")

# callback function for the upcoming messages
def on_message(client, userdata, msg):
    global channelsData, deviceType, deviceDACmode
    # check if the message is an Alarm message
    if(not isDeviceDacModeAlarm(msg.payload)):
        return
    deviceType, deviceDACmode, channelName, channelDateTime, alarmStatus, channelData = parsePayloadBeandevice(msg.payload)
    # store data
    for element in channelsData:
        if element["channelName"] == channelName:
            element["alarmStatus"] = alarmStatus
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
