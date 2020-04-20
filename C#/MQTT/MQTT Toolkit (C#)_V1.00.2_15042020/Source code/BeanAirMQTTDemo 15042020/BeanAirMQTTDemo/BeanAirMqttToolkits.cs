using System;
using System.Collections.Generic;
using System.Net.Sockets;
using System.Text;
using System.Windows.Forms;
using System.Threading;
using uPLibrary.Networking.M2Mqtt;
using uPLibrary.Networking.M2Mqtt.Exceptions;
using uPLibrary.Networking.M2Mqtt.Messages;
using System.Net;
using System.Collections.Concurrent;
using System.IO;
using System.Drawing;

namespace BeanAirMQTTDemo
{

    public partial class BeanAirMqttToolkits : Form
    {
        public MQTTProcessState mQTTProcessState = MQTTProcessState.Stopped;

        MQTTStreaming handler = new MQTTStreaming();
        private const string DATE_FORMAT = "yyyy/MM/dd HH:mm:ss.fff";
        private const string DATE_FORMAT_FILE = "yyyy_MM_dd_HH_mm_ss_fff";
        /// <summary>
        /// 10 Mbytes
        /// </summary>
        public long LOG_MAX_SIZE
        {
            get
            {
                return (int)numericUpDownMaxPartSize.Value * (1024 * 1024);
            }
        }
        DateTime currentIncomingDate = DateTime.MinValue;
        public static int PartNumber = 0;
        public static long ClientSubscribeId = DateTime.Now.Ticks;


        public BeanAirMqttToolkits()
        {
            InitializeComponent();
            InitVersion();
            TxtBx_Subscribe.Text = "MAC_ID/STREAMING";


            ChkBx_DNS.Checked = false;
            UpdateView(false);
            UpdateEntry();
            InitDurationManagement();

        }

        private void InitVersion()
        {
            try
            {
                this.toolStripStatusLabel1.Text = string.Format("Copyright © BeanAir {0} , Version : {1} , Release Date : {2}"
                       , DateTime.Now.Year
                       , Application.ProductVersion.ToString()
                           , "15/04/2020"
                       );
            }
            catch (Exception)
            {

            }
        }
        private void LOG(string msg)
        {
            //System.Diagnostics.Debug.WriteLine(" DEVELOPER_DEBUG_MESSAGE : " + msg);
        }
        #region  DataReading

        private System.Threading.Timer streamingLogTimer;
        int timer_Count = 0;
        int cycle = 0;
        int duration = 0;
        void InitDurationManagement()
        {
            cycle = 60;
            duration = 10;
            checkBoxDataReading.Checked = true;
            textBox_dataReadingCycle.Text = this.cycle.ToString();
            textBox_datareadingDuration.Text = this.duration.ToString();
        }
        private void buttonDataReading_Click(object sender, EventArgs e)
        {

            if (checkBoxDataReading.Checked)
            {
                int _cycle = (int)this.numericUpDown_Cycle.Value;
                int _duration = (int)this.numericUpDown_duration.Value;
                if (_cycle > _duration)
                {
                    this.cycle = _cycle;
                    this.duration = _duration;
                }
                else
                {
                    MessageBox.Show("Data reading cycle should be higher than thez data reading duration");
                }
            }
            else
            {
                this.cycle = 0;
                this.duration = 0;
            }

            textBox_dataReadingCycle.Text = this.cycle.ToString();
            textBox_datareadingDuration.Text = this.duration.ToString();
        }


        private void StartTimer()
        {
            streamingLogTimer = new System.Threading.Timer(timerCallBack);
            streamingLogTimer.Change(1000, Timeout.Infinite); /** start 5 seconds after the type constructor */
            LOG("Start timer");
        }
        private void StopTimer()
        {
            if (streamingLogTimer != null)
            {
                streamingLogTimer.Change(Timeout.Infinite, Timeout.Infinite); /** start 5 seconds after the type constructor */
                timer_Count = 0;
            }
        }


        private void timerCallBack(object state)
        {

            if (timer_Count < this.cycle)
            {
                timer_Count++;
            }
            else
            {
                timer_Count = 0;
            }

            LOG("timer_Count = " + timer_Count);
            streamingLogTimer.Change(1000, Timeout.Infinite);
        }





        #endregion
        #region MQTT_Manager

        #region fileds
        private MqttClient mqttClient = null;
        string Topic = string.Empty;



        #endregion

        public bool CreateMQTTClient()
        {
            if (!IsConnected)
            {
                CreateMQTTClientInstance();
                SubscribeToEvent();
            }
            return IsConnected;
        }

        internal bool StartMQTT()
        {
            var res = false;
            if (IsConnected && IsThreadLaunched(this.IncomingMessageThread))
                res = true;
            else
            {
                if (CreateMQTTClient())
                {
                    StartProsessing();
                    res = true;
                }
            }
            mQTTProcessState = res ? MQTTProcessState.Connetion_Success
                                   : MQTTProcessState.Connetion_Failure;
            UpdateSate();
            return res;
        }

        private void SubscribeToEvent()
        {
            if (IsConnected)
            {
                UnSubscribeFromEvent();

                mqttClient.ConnectionClosed -= Client_ConnectionClosed;
                mqttClient.MqttMsgPublishReceived += Client_recievedMessage;
                mqttClient.ConnectionClosed += Client_ConnectionClosed;


            }
        }
        private void Client_ConnectionClosed(object sender, EventArgs e)
        {

            if (this.mqttClient != null)
            {
                mqttClient.ConnectionClosed -= Client_ConnectionClosed;
                this.mqttClient = null;
            }
            StopMQTT();
            //  MessageBox.Show("MQTT Connection Closed  Successfully ");

        }
        public bool UnLoadMQTTClient()
        {
            try
            {
                if (mqttClient != null)
                {
                    UnSubscribeFromEvent();
                    Unsbscribe();
                    byte retry = 0;
                    while (IsConnected && retry < 2)
                    {
                        mqttClient.Disconnect();
                        Thread.Sleep(500);
                        retry++;
                    }
                }
                return true;
            }
            catch (Exception exx)
            {
                return false;
            }
        }

        private void UnSubscribeFromEvent()
        {
            try
            {
                if (mqttClient != null)
                    mqttClient.MqttMsgPublishReceived -= Client_recievedMessage;
            }
            catch (uPLibrary.Networking.M2Mqtt.Exceptions.MqttClientException e)
            { }
        }



        public bool IsConnected
        {
            get { return mqttClient != null ? mqttClient.IsConnected : false; }
        }

        #region Event


        private void Client_recievedMessage(object sender, MqttMsgPublishEventArgs e)
        {
            try
            {
                if (this.cycle == this.duration || timer_Count < duration)
                {
                    if (e != null) IncomingMessage.Enqueue(e);
                }
            }
            catch (Exception ex)
            {
                LOG("ERROR : " + ex.Message);
            }
        }
        #endregion

        #region Connect


        private bool CreateMQTTClientInstance()
        {
            mqttClient = null;
            if (CreateInstance() && mqttClient != null)
                ConnectToBrocker();
            //if (IsConnected)
            //    //  MessageBox.Show("MQTT Connetion  SUCCESS");
            return IsConnected;
        }
        private bool CreateInstance()
        {
            try
            {
                try
                {
                    if (Settings.MQTT_Broker_DnsFlag)//Use DNS
                    {
                        mqttClient = new MqttClient(Settings.MQTT_Broker_Dns,
                            Settings.MQTT_Broker_Port,
                           false, null, null, MqttSslProtocols.None);
                    }
                    else//Use IP Adress
                    {
                        IPAddress MQTT_Broker_IP = IPAddress.Parse(Settings.MQTT_Broker_IP);
                        mqttClient = new MqttClient(MQTT_Broker_IP, Settings.MQTT_Broker_Port,
                            false, null, null, MqttSslProtocols.None);
                    }
                    return true;
                }
                catch (SocketException e)
                {
                    MessageBox.Show("Check host name or ip");
                    return false;
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message); return false;
            }
        }
        private bool ConnectToBrocker()
        {
            try
            {
                mqttClient.Connect(Guid.NewGuid().ToString());
            }
            catch (MqttConnectionException e)
            {
                var msg = "Cannot connect to the server , chech Params";
                MessageBox.Show(msg + " __ " + e.Message);
                return false;
            }
            catch (Exception exx)
            {
                MessageBox.Show("MQTT Connetion " + exx.Message);
                return false;

            }
            return IsConnected;

        }

        #endregion





        private void BtnValidate_BrokerSetting_Click(object sender, EventArgs e)
        {
            try
            {
                if (!IsConnected)
                {
                    #region mqtt config
                    Settings.MQTT_Broker_Port = int.Parse(TxtBx_cnfPort.Text);
                    Settings.MQTT_Broker_IP = TxtBx_cnfBrokerIp.Text;
                    Settings.MQTT_Broker_Dns = txtBx_cnfDNS.Text;
                    Settings.MQTT_Broker_DnsFlag = ChkBx_DNS.Checked;
                    #endregion

                    //Update
                    UpdateEntry();


                    MessageBox.Show("MQTT config Saved");
                }
                else
                    MessageBox.Show("MQTT Modul should be stopped");
            }
            catch (Exception ex)
            {
                MessageBox.Show("Check fileds entry", "Brocker setting",
                  MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }
        public void UpdateEntry()
        {
            TxtBx_Port.Text = Settings.MQTT_Broker_Port.ToString();
            TxtBx_BrokerIp.Text = Settings.MQTT_Broker_IP.ToString();
            TxtBx_DNS.Text = Settings.MQTT_Broker_Dns.ToString();
            ChkBx_DNS.Checked = Settings.MQTT_Broker_DnsFlag;

        }



        #endregion
        private void buttonConnect_Click(object sender, EventArgs e)
        {
            StartMQTT();
        }
        private void buttonDisconnect_Click(object sender, EventArgs e)
        {
            StopMQTT();
            UpdateSate();
        }

        private void UpdateSate()
        {
            var stateStr = "MQTT Status  : ";
            switch (this.mQTTProcessState)
            {
                case MQTTProcessState.Init:
                    stateStr += "Init"; break;
                case MQTTProcessState.Started:
                    stateStr += "Started"; break;
                case MQTTProcessState.Processing:
                    stateStr += "Processing"; break;
                case MQTTProcessState.Completed:
                    stateStr += "Completed"; break;
                case MQTTProcessState.Stopped:
                    stateStr += "Stopped"; break;
                case MQTTProcessState.Connetion_Success:
                    stateStr += "Connetion Success"; break;
                case MQTTProcessState.Connetion_Failure:
                    stateStr += "Connetion Failure"; break;
                default: break;
            }
            textBoxState.Text = stateStr;
        }
        internal bool StopMQTT()
        {
            var res = false;
            try
            {
                res = UnLoadMQTTClient();
                if (res)
                {
                    if (this.IncomingMessageThread != null)
                    {
                        this.IncomingMessageThread.Suspend();
                        this.IncomingMessageThread.Resume();
                        this.IncomingMessageThread.Abort();
                        this.IncomingMessageThread = null;

                        mQTTProcessState = MQTTProcessState.Stopped;

                        // free the buffer 
                        MqttMsgPublishEventArgs e;
                        while (IncomingMessage.Count > 0)
                        {
                            IncomingMessage.TryDequeue(out e);
                            //label_MsgInQueue.Text = "" + IncomingMessage.Count;
                        }
                    }
                }
            }
            catch (Exception) { }
            return res;
        }
        private void ErrMsg(string Msg)
        {
            MessageBox.Show(Msg, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
        }
        private void Btn_Subscribe_Click(object sender, EventArgs e)
        {
            SubscribeToEvent();
            ClientSubscribeId = DateTime.Now.Ticks;
            PartNumber = 0;
            InitViewResult();
            if (!IsConnected)
            {
                ErrMsg("MQTT Not connected");
                return;
            }
            if (string.IsNullOrEmpty(TxtBx_Subscribe.Text))
            {
                ErrMsg("Enter topic Name");
                return;
            }
            try
            {

                mqttClient.Subscribe(new string[] { TxtBx_Subscribe.Text }, new byte[] { MqttMsgBase.QOS_LEVEL_AT_MOST_ONCE });
                Topic = TxtBx_Subscribe.Text;
                textBoxTopicState.Text = "Subscribe to " + Topic;
                Settings.TOPIC = Topic;
                this.Btn_Subscribe.Enabled = false;
                this.buttonDataReading.Enabled = false;
                checkBox_Is_DeviceVersionHigherOrEqualThanV3R8.Enabled = false;
                StartTimer();

            }
            catch (Exception exx)
            {
                textBoxTopicState.Text = "Subscribe Failure to " + Topic;
                //   MessageBox.Show("Subscribe Failure");
            }
        }
        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            UnLoadMQTTClient();
        }



        #region Processing

        private void StartProsessing()
        {

            StartThread_IncomingMessage();
        }
        private bool IsThreadLaunched(Thread thread)
        {
            return thread != null && (
                   thread.IsAlive ||
                   thread.ThreadState == ThreadState.Running);
        }

        private void StartThread_IncomingMessage()
        {

            bool isWork = this.IncomingMessageThread != null
                && (this.IncomingMessageThread.IsAlive || this.IncomingMessageThread.ThreadState == ThreadState.Running);
            if (isWork == false)
            {
                try
                {
                    this.IncomingMessageThread = new Thread(new ThreadStart(this.StartProsess_IncomingMessage));
                    this.IncomingMessageThread.Name = "MqttThread";
                    this.IncomingMessageThread.Start();
                }
                catch (Exception ee)
                {

                }
            }


        }

        private void InitViewResult()
        {

            UpdateTextBox(textBoxDateMesaure, "NA");
            UpdateTextBox(textBox_SR, "NA");
            UpdateTextBox(textBox_ActiveSensor, "NA");
            UpdateTextBox(textBox_segmentNumber, "NA");
            UpdateTextBox(textBox_NumberOfDaqPerChannel, "NA");
            UpdateTextBox(textBox_measureCycle_Notif, "NA");
            UpdateTextBox(textBox_acqDuration, "NA");
            UpdateTextBox(textBox_Lqi_Value, "NA");
            UpdateTextBox(textBoxSynchro, "NA");
            UpdateTextBox(textBox_MathModeDateStart, "NA");
            UpdateTextBox(textBox_MathModeDateEnd, "NA");
            listView1.BeginInvoke(new Action(() => { listView1.Items.Clear(); }));
            listViewMeasures.BeginInvoke(new Action(() => { listViewMeasures.Items.Clear(); }));
        }
        Thread IncomingMessageThread = null;
        private ConcurrentQueue<MqttMsgPublishEventArgs> IncomingMessage = new ConcurrentQueue<MqttMsgPublishEventArgs>();

        private void StartProsess_IncomingMessage()
        {
            this.mQTTProcessState = MQTTProcessState.Started;

            while (mQTTProcessState != MQTTProcessState.Stopped)
            {

                // Dequeue
                if (IncomingMessage.Count > 0
                    && (mQTTProcessState == MQTTProcessState.Started ||
                      mQTTProcessState == MQTTProcessState.Completed))
                {
                    MqttMsgPublishEventArgs e;
                    if (IncomingMessage.TryDequeue(out e))
                    {
                        mQTTProcessState = MQTTProcessState.Processing;
                        if (e != null && e.Message.Length > 0)
                        {
                            Process_IncomingMessage(e);
                        }
                        //label_MsgInQueue.BeginInvoke(new Action(() =>
                        //        {
                        //            label_MsgInQueue.Text = IncomingMessage.Count.ToString();
                        //        }));
                        mQTTProcessState = MQTTProcessState.Completed;
                    }
                }
                // Thread.Sleep(50);
            }
        }

        private void Process_IncomingMessage(MqttMsgPublishEventArgs e)
        {
            try
            {
                handler.Is_DeviceVersionHigherOrEqualThanV3R8 = checkBox_Is_DeviceVersionHigherOrEqualThanV3R8.Checked;
                var measureModeType = handler.BuildHeader(e.Message);

                switch (measureModeType)
                {
                    case MeasureModeType.Streaming:
                    case MeasureModeType.ShockDetectEventMode:
                    case MeasureModeType.StreamingAlarm:
                        var model = handler.StreaminSynchronizationFrame(e.Message);
                        model.Topic = e.Topic;
                        if (currentIncomingDate != model.StreamInfoNewComingDate)
                        {
                            currentIncomingDate = model.StreamInfoNewComingDate;
                            InitViewResult();
                        }


                        UpdateView(model);
                        break;
                    case MeasureModeType.DYNAMIC_MATHMODE:
                        var mathmodel = handler.BuildBody_MathMode(e.Message, e.Topic);

                        UpdateMathView(mathmodel);
                        break;

                }

            }
            catch (Exception ex)
            {
                var msg = ex.Message;
            }
        }


        private void UpdateMathView(MathDataModel mathmodel)
        {
            UpdateTextBox(textBox_MathModeDateStart, mathmodel.DateStart.ToString());
            UpdateTextBox(textBox_MathModeDateEnd, mathmodel.DateEnd.ToString());
            ListViewItem[] items = new ListViewItem[mathmodel.List_Sensors.Count];
            int i = 0;
            foreach (var sensor in mathmodel.List_Sensors)
            {
                var array = new string[6];
                array[0] = sensor.SensLabelStr;
                array[1] = sensor.MathModeResult_MinValueDate.ToString();
                array[2] = sensor.MathModeResult_MinValue.ToString();
                array[3] = sensor.MathModeResult_MaxValueDate.ToString();
                array[4] = sensor.MathModeResult_MaxValue.ToString();
                array[5] = sensor.MathModeResult_Average.ToString();
                items[i] = new ListViewItem(array);
                i++;
            }
            listView1.BeginInvoke(new Action(() =>
            {
                listView1.Items.Clear();
                listView1.Items.AddRange(items);
            }));
        }

        private void UpdateView(StreamingDataModel streamingDataModel)
        {
            if (streamingDataModel != null && streamingDataModel.StreamInfoNewComingDate > DateTime.MinValue)
            {
                var dateStartMeasurements = streamingDataModel.StreamInfoNewComingDate;

                #region UpdateTextbox

                var prevDate = textBoxDateMesaure.Text;
                if (prevDate != dateStartMeasurements.ToString(DATE_FORMAT))
                {
                    UpdateTextBox(textBoxDateMesaure, dateStartMeasurements.ToString(DATE_FORMAT));
                    UpdateTextBox(textBox_SR, streamingDataModel.SamplingFrequency.ToString());
                    UpdateTextBox(textBox_ActiveSensor, streamingDataModel.NumberOfActivSensors.ToString());
                    UpdateTextBox(textBox_measureCycle_Notif, streamingDataModel.MeasureCycle_Notif.ToString());
                    UpdateTextBox(textBox_acqDuration, streamingDataModel.AcqDuration.ToString());
                }
                UpdateTextBox(textBox_NumberOfDaqPerChannel, streamingDataModel.NumberOfDaqPerChannel.ToString());
                UpdateTextBox(textBox_segmentNumber, streamingDataModel.SegmentNumber.ToString());
                UpdateTextBox(textBox_Lqi_Value, streamingDataModel.Lqi_Value.ToString());
                if (checkBox_Is_DeviceVersionHigherOrEqualThanV3R8.Checked)
                    UpdateTextBox(textBoxSynchro, streamingDataModel.IsSynchronized.ToString());
                else
                    UpdateTextBox(textBoxSynchro, "NA");
                #endregion

                var _Measures = new List<List<double>>();
                var _Dates = new List<DateTime>();
                var _Index = new List<long>();
                var refDate = this.handler.CurrentStreaming_Date;
                var sensoridList = this.handler.SensIdList;

                this.handler.ExtractMeasurement2(out _Measures, out _Dates, out _Index);
                if (checkBoxScreenLog.Checked)
                {
                    var displayThread = new Thread(new ThreadStart(() => { Work(_Index, _Measures); }));
                    // start the thread  
                    displayThread.Start();

                }
                if (checkBoxLogFolder.Checked)
                {
                    AppendData(streamingDataModel, _Index, _Measures);
                }


                //if (checkBoxScreenLog.Checked)
                //{
                //    var displayThread = new Thread(new ThreadStart(() => { Work(Measurements); }));
                //    // start the thread  
                //    displayThread.Start();
                //}
            }
        }

        private void Work(List<long> _Index, List<List<double>> _Measures)
        {
            int MAX_ROWS = 10000;
            try
            {
                var sensorCount = _Measures.Count;
                var columnCount = sensorCount + 1;
                int total = _Index.Count;
                int startIndex = total - 1;
                for (int measureIndex = 0; measureIndex < _Index.Count; measureIndex++)
                {
                    var array = new string[columnCount];
                    array[0] = _Index[measureIndex].ToString();
                    for (int sensorIndex = 0; sensorIndex < sensorCount; sensorIndex++)
                        array[sensorIndex + 1] = _Measures[sensorIndex][measureIndex].ToString();
                    listViewMeasures.BeginInvoke(new Action(() =>
                    {
                        int count = listViewMeasures.Items.Count;
                        if (count > MAX_ROWS)
                        {
                            listViewMeasures.Items.RemoveAt(MAX_ROWS);
                        }
                        listViewMeasures.Items.Insert(0, new ListViewItem(array));
                    }));
                }
            }
            catch (Exception ex)
            {
                var msg = ex.Message;
            }
        }

        private void UpdateTextBox(TextBox textBox, string Text)
        {
            try
            {
                if (textBox.InvokeRequired)
                {
                    textBox.BeginInvoke(new Action(() =>
                    {
                        textBox.Text = Text;
                    }));
                }
                else
                {
                    textBox.Text = Text;
                }
            }
            catch (Exception)
            {

            }
        }


        #endregion
        private void Unsbscribe()
        {
            if (Topic != String.Empty)
            {
                this.Btn_Subscribe.Enabled = true;
                this.buttonDataReading.Enabled = true;
                mqttClient.Unsubscribe(new string[] { Topic });
                textBoxTopicState.Text = "NO TOPIC";
                checkBox_Is_DeviceVersionHigherOrEqualThanV3R8.Enabled = true;

            }
        }
        private void buttonUnsubscribe_Click(object sender, EventArgs e)
        {
            Unsbscribe();
            StopTimer();
        }

        private void AppendData(StreamingDataModel streamingDataModel, List<long> _Index, List<List<double>> _Measures)

        {
            try
            {
                var fileNameStart = "TX_";
                string data = String.Empty;
                String directory = Settings.SUB_LOG_DIRECTORY;

                if (directory.Length > 0)
                {
                    if (!Directory.Exists(directory))
                    {
                        Directory.CreateDirectory(directory);
                    }
                    #region BuilData
                    var sensorlist = this.handler.SensIdList;
                    var str = new StringBuilder();
                    if (_Index != null && _Measures != null && sensorlist != null
                        && _Measures.Count == sensorlist.Count
                        && _Measures[0].Count == _Index.Count)
                    {

                        for (int measureIndex = 0; measureIndex < _Index.Count; measureIndex++)
                        {
                            //0;2.0;.3;...
                            //n_index;2.0;.3;...
                            long currentIndex = _Index[measureIndex];
                            string line = currentIndex.ToString();
                            for (int sensorindex = 0; sensorindex < sensorlist.Count; sensorindex++)
                            {
                                double measure = _Measures[sensorindex][measureIndex];
                                line += ";" + measure;
                            }
                            str.AppendLine(line);
                        }
                    }
                    data = str.ToString();
                    #endregion

                    var _fileInfo = GetFileInfo(streamingDataModel.StreamInfoNewComingDate, directory);
                    if (_fileInfo != null)
                    {
                        if (_fileInfo.Exists)
                        {
                            using (TextWriter tw = _fileInfo.AppendText())
                                tw.Write(data);
                        }
                        else
                        {
                            using (TextWriter tw = _fileInfo.CreateText())
                            {
                                if (_fileInfo.FullName.Contains("_part") == false)
                                    tw.WriteLine(BuildFileHeader(streamingDataModel));
                                tw.Write(data);
                            }
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                var msg = ex.Message;
            }
        }

        private FileInfo GetFileInfo(DateTime _RefDate, string directory)
        {
            FileInfo _fileInfo = null;
            try
            {

                var fileNameStart = "TX_";
                string _part = "";
                string date_file = _RefDate.ToString(DATE_FORMAT_FILE);

                if (PartNumber > 0)
                    _part = "_part" + PartNumber;
                string fileName = directory + "\\" + fileNameStart + date_file + _part + ".txt";
                bool x = false;
                while (x == false)
                {
                    _fileInfo = new FileInfo(Path.Combine(@directory, @fileName));

                    if (_fileInfo != null && _fileInfo.Exists && _fileInfo.Length >= LOG_MAX_SIZE)
                    {
                        PartNumber++;
                        fileName = directory + "\\" + fileNameStart + date_file + "_part" + PartNumber + ".txt";
                    }
                    else
                    {
                        x = true;
                    }

                }
            }
            catch (Exception ex)
            {
                var msg = ex.Message;
            }

            return _fileInfo;

        }

        private string BuildFileHeader(StreamingDataModel streamingDataModel)
        {
            var _header = new StringBuilder();
            /* write header into the new file */
            _header.AppendLine("---------------------------------------------------------------------");
            _header.AppendLine("Topic : " + Topic);
            _header.AppendLine("Date : " + streamingDataModel.StreamInfoNewComingDate.ToString(DATE_FORMAT));
            _header.AppendLine("Sampling Frequency(hz) : " + streamingDataModel.SamplingFrequency);
            _header.AppendLine("Daq acq.Cycle(sec) : " + streamingDataModel.MeasureCycle_Notif);
            _header.AppendLine("Daq acq.Duration(sec) : " + streamingDataModel.AcqDuration);
            _header.AppendLine("---------------------------------------------------------------------");
            _header.AppendLine(buildHeader(streamingDataModel.SensIdList));

            return _header.ToString();
        }
        private string buildHeader(List<byte> sensIdList)
        {
            try
            {
                var idListStr = "";
                int i = 0;
                foreach (var id in sensIdList)
                {
                    if (i == 0)
                        idListStr += " Ch_Id_" + id;
                    else
                        idListStr += ";Ch_Id_" + id;
                    i++;
                }
                var header = "Index; Measures " + idListStr;
                return header;
            }
            catch (Exception)
            {
                return " ";
            }
        }
        private void button1_Click(object sender, EventArgs e)
        {
            if (folderBrowserDialog1.ShowDialog() == DialogResult.OK)
            {
                this.textBox_Directory.Text = folderBrowserDialog1.SelectedPath;
                Settings.LOG_DIRECTORY = folderBrowserDialog1.SelectedPath;
            }
        }

        private void checkBoxLogFolder_CheckedChanged(object sender, EventArgs e)
        {
            var state = checkBoxLogFolder.Checked;
            textBox_Directory.Enabled = state;
            buttonBrowse.Enabled = state;
        }

        private void ChkBx_DNS_CheckedChanged(object sender, EventArgs e)
        {
            bool isChecked = ChkBx_DNS.Checked;

            UpdateView(isChecked);
        }

        private void UpdateView(bool isChecked)
        {
            TxtBx_DNS.Enabled = isChecked;
            txtBx_cnfDNS.Enabled = isChecked;

            TxtBx_BrokerIp.Enabled = !isChecked;
            TxtBx_cnfBrokerIp.Enabled = !isChecked;
        }




    }
}