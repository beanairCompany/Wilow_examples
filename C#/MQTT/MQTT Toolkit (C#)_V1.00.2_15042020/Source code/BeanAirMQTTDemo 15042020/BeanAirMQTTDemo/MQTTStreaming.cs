using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Windows;

namespace BeanAirMQTTDemo
{

    public enum MeasureModeType : byte
    {
        None = 0x00,
        Streaming = 0x03,
        StreamingAlarm = 0x05,
        ShockDetectEventMode = 0x06,
        DYNAMIC_MATHMODE = 0x07,
    }
    public enum DaqAcqMode_FrameOption : byte
    {
        No_specific_DAQ_mode_farme_options = 0x00,
        First_Frame = 0x10,
        Raw_Stream_Data_with_frame_size_lower_than_127_bytes = 0x20,
        Raw_Stream_Data_with_frame_size_higher_than_127_bytes = 0x30,
        Raw_Data_for_static_data_acquisition = 0x40,
        Math_results = 0x50,
    }

    public class MQTTStreaming
    {

        private const int MIN_MESSAGE_SIZE = 20;
        private int calculatedPayDataLen = 20;
        private UInt32 segmentNumber;
        private int dynamic = 3;
        private int NumberOfDaqPerChannel;
        private int NumberOfDaqPerChannel_Prev;
        private UInt32[,] measureDataPerSensor;
        private UInt16 samplingFrequency;
        UInt32 sensorEnabledBitmap;
        private byte numberOfActivSensors;
        private List<Byte> sensIdList = new List<byte>();
        public static int x = 0;
        DateTime streamInfoNewComingDate = DateTime.MinValue;
        DateTime laststreamInfoComingDate = DateTime.MinValue;
        private int DataLength = 0;
        private UInt16 Millisecond = 0;
        private int DYNAMIC_HEADER_LENGTH = 0;
        bool is_DeviceVersionHigherOrEqualThanV3R8 = true;
        byte[] Message = new byte[] { };
        protected byte message_Type = new byte();
        protected byte device_Type = new byte();
        protected byte acquisition_Type = new byte();
        protected byte sensor_measure_Mode = new byte();
        protected byte sensor_measure_Mode_option = 0x00;
        protected MeasureModeType measureModeType = MeasureModeType.None;
        private byte[] shockvalue = new byte[6];

        public List<Byte> SensIdList
        {
            get
            {
                return sensIdList;
            }
        }
        public  bool   Is_DeviceVersionHigherOrEqualThanV3R8 {
            get { return is_DeviceVersionHigherOrEqualThanV3R8; }
            set { is_DeviceVersionHigherOrEqualThanV3R8 = value; } }
        public MeasureModeType BuildHeader(byte[] incomingMessage)
        {
            if (incomingMessage.Length >= MIN_MESSAGE_SIZE)
            {
                this.Message = incomingMessage;


                this.device_Type = incomingMessage[0];
                ///acquisition_Type contain the measure mode OR an indication that it's a math result
                this.acquisition_Type = incomingMessage[1];
                switch (this.acquisition_Type)
                {
                    case 0x03: this.measureModeType = MeasureModeType.Streaming; break;
                    case 0x04: this.measureModeType = MeasureModeType.ShockDetectEventMode; break;
                    case 0x06: this.measureModeType = MeasureModeType.StreamingAlarm; break;
                    case 0x07: this.measureModeType = MeasureModeType.DYNAMIC_MATHMODE; break;
                }
                if (this.measureModeType == MeasureModeType.None)
                {
                    AddLog("Wrong MeasureModeType, mqtt streaming,Set,Shock Analyser");
                }
                return this.measureModeType;
            }
            return MeasureModeType.None;
        }


        private void AddLog(string v)
        {
            Trace.TraceInformation(v);
        }
        public MathDataModel BuildBody_MathMode(byte[] incomingMessage, string Topic)
        {

            try
            {
                var model = new MathDataModel();
                model.Topic = Topic;
                int dataIndex = 2;
                byte sensor_measure_Mode_option = incomingMessage[dataIndex];
                var _DaqAcqMode_FrameOption = (DaqAcqMode_FrameOption)sensor_measure_Mode_option;
                T("_DaqAcqMode_FrameOption : " + _DaqAcqMode_FrameOption);
                dataIndex++;
                model.SamplingFrequency = BitConverter.ToInt16(incomingMessage, dataIndex);
                dataIndex += 2;

                // measureCycle_Notif;//3byte
                byte[] AUX_measureCycle_Notif = new byte[4];
                Array.Copy(incomingMessage, dataIndex, AUX_measureCycle_Notif, 0, 3);
                model.MeasureCycle_Notif = BitConverter.ToUInt32(AUX_measureCycle_Notif, 0);
                dataIndex += 3;
                // acqDuration;//3byte 
                byte[] AUX_acqDuration = new byte[4];
                Array.Copy(incomingMessage, dataIndex, AUX_acqDuration, 0, 3);
                model.AcqDuration = BitConverter.ToUInt32(AUX_acqDuration, 0);
                dataIndex += 3;
                /*
                 * Future Use
                 */
                byte future_use = incomingMessage[dataIndex];
                dataIndex++;

                model = StaticMathModeFrame.ExtractMathModeData(model, dataIndex, incomingMessage);

                return model;
            }
            catch (Exception ex)
            {
                var msg = ex.Message;
                T("exp in read math frame mqtt : msg = " + msg);
            }
            return null;
        }

        private void T(string p)
        {
            Trace.TraceInformation(p);
        }

        public StreamingDataModel StreaminSynchronizationFrame(byte[] incomingMessage)
        {

            //Wrong Message
            if (incomingMessage.Length < MIN_MESSAGE_SIZE)
                return null;
            var model = new StreamingDataModel();
            int dataIndex = 2;
            this.Message = incomingMessage;

            //--ReferenceTime  4 Byte startIndex=2
            byte[] AUX_DateInUnixTimeFormat = new byte[4];
            Array.Copy(this.Message, dataIndex, AUX_DateInUnixTimeFormat, 0, 4);
            streamInfoNewComingDate = GetTime(AUX_DateInUnixTimeFormat);
            dataIndex += 4;

            // Millisecond =2 Byte startIndex=4
            this.Millisecond = 0;/* should be initialised*/
            //if (this.SOFT_VERS >= BeanComConfig.MIN_SOFT_VERS_SUPPORT_Milliseconds)
            //{
            Millisecond = BitConverter.ToUInt16(incomingMessage, dataIndex);
            dataIndex += 2;
            // }
            streamInfoNewComingDate = streamInfoNewComingDate.AddMilliseconds(this.Millisecond);
            model.StreamInfoNewComingDate = streamInfoNewComingDate;

            if (laststreamInfoComingDate != streamInfoNewComingDate)
            {
                laststreamInfoComingDate = streamInfoNewComingDate;
                BeanAirMqttToolkits.PartNumber = 0;
                System.Diagnostics.Trace.TraceInformation("PartNumber =  " + BeanAirMqttToolkits.PartNumber);
            }

            //--SamplingFreq 2 Byte  startIndex=6
            this.samplingFrequency = BitConverter.ToUInt16(this.Message, dataIndex);
            model.SamplingFrequency = samplingFrequency;



            dataIndex += 2;
            //--sensorEnabledBitmap    Byte startIndex=8
            this.sensorEnabledBitmap = BitConverter.ToUInt32(this.Message, dataIndex);
            dataIndex += 4;
            sensIdList.Clear();
            for (byte i = 0; i < 5; i++)
            {
                if ((sensorEnabledBitmap & (1 << i)) == (1 << i))
                    sensIdList.Add(i);
            }
            model.SensIdList = sensIdList;


            ///numberOfActivSensors
            this.numberOfActivSensors = (byte)sensIdList.Count;

            model.NumberOfActivSensors = numberOfActivSensors;

            //-- SegmentNumber   Byte startIndex=12
            byte[] FrameSeqId = new byte[4];
            Array.Copy(this.Message, dataIndex, FrameSeqId, 0, 3);
            segmentNumber = BitConverter.ToUInt32(FrameSeqId, 0);
            dataIndex += 3;
            model.SegmentNumber = segmentNumber;


            ////NumberOfDaqPerSensor 2 Byte startIndex=15 , NumberOfDaqPerSensor its the number of packet
            NumberOfDaqPerChannel = BitConverter.ToUInt16(this.Message, dataIndex);
            dataIndex += 2;
            model.NumberOfDaqPerChannel = NumberOfDaqPerChannel;

            // measureCycle_Notif;//3byte --- ToDo show notif device working well but there is no alarm s.e.t

            byte[] AUX_measureCycle_Notif = new byte[4];
            Array.Copy(incomingMessage, dataIndex, AUX_measureCycle_Notif, 0, 3);
            var measureCycle_Notif = BitConverter.ToUInt32(AUX_measureCycle_Notif, 0);
            dataIndex += 3;

            model.MeasureCycle_Notif = measureCycle_Notif;


            // acqDuration;//3byte

            byte[] AUX_acqDuration = new byte[4];
            Array.Copy(incomingMessage, dataIndex, AUX_acqDuration, 0, 3);
            var acqDuration = BitConverter.ToUInt32(AUX_acqDuration, 0);
            dataIndex += 3;

            model.AcqDuration = acqDuration;




            byte[] AUX_NumberOfDaqPerChannel_Prev = new byte[2];
            Array.Copy(incomingMessage, dataIndex, AUX_NumberOfDaqPerChannel_Prev, 0, 1);
            NumberOfDaqPerChannel_Prev = BitConverter.ToUInt16(AUX_NumberOfDaqPerChannel_Prev, 0);
            dataIndex += 2;
            if (Is_DeviceVersionHigherOrEqualThanV3R8)
            {
                model.TimeSynchronization = incomingMessage[dataIndex];
                dataIndex++;
            }
            else // these are a freee byte  
            {
                dataIndex++;/// free for timeSynchronization
            }

            //Lqi_Value;//1byte
            model.Lqi_Value = incomingMessage[dataIndex];
            dataIndex += 1;



            if (this.measureModeType == MeasureModeType.StreamingAlarm)
            {
                var FirstChannelAlarmDetected = incomingMessage[dataIndex];
                //BeanLog.Log.Instance.AddLog("FirstChannelAlarmDetected : " + FirstChannelAlarmDetected);
                dataIndex += 1;
            }

            /*_NEW_*/

            if (this.measureModeType == MeasureModeType.ShockDetectEventMode)//case of shock detection
            {

                var shockEventSource = incomingMessage[dataIndex++];
                Array.Copy(incomingMessage, dataIndex, shockvalue, 0, 6);//Schock detection first schock value
                dataIndex += 6;
            }


            DataLength = dynamic// 3byte for each measure
                         * this.numberOfActivSensors
                         * NumberOfDaqPerChannel;

            DYNAMIC_HEADER_LENGTH = dataIndex;


            ////// END UPDATING DATA 
            if (CurrentStreaming_Date != streamInfoNewComingDate)//exist but a new one occured
                CurrentStreaming_Date = streamInfoNewComingDate;
            var measureNumber = DataLength;
            measureNumber /= dynamic * this.numberOfActivSensors;
            calculatedPayDataLen = (measureNumber * dynamic * this.numberOfActivSensors);

            L("NumberOfDaqPerChannel: " + NumberOfDaqPerChannel + "NumberOfDaqPerChannel_Prev : " + NumberOfDaqPerChannel_Prev);

            /* Check data validity with calculated data length */
            if (calculatedPayDataLen != DataLength)
                return null;
            return model;
        }

        private void L(string v)
        {
            Trace.TraceInformation(v);
        }

        private UInt32[,] TransformStreamMeasureDataBlock(byte[] byteArray, int dataIndex, int sensorCount, int measureCount, int dynamic)
        {
            if (byteArray == null ||
                byteArray.Length + dataIndex < sensorCount * measureCount * dynamic)
            {
                return null;
            }

            UInt32[,] measureDataPerSensor = new UInt32[sensorCount, NumberOfDaqPerChannel];

            for (int measureIndex = 0; measureIndex < NumberOfDaqPerChannel; measureIndex++)
            {
                for (int sensorIndex = 0; sensorIndex < this.numberOfActivSensors; sensorIndex++)
                {
                    switch (dynamic)
                    {
                        case 1:
                            measureDataPerSensor[sensorIndex, measureIndex] = byteArray[dataIndex];
                            break;
                        case 2:
                            measureDataPerSensor[sensorIndex, measureIndex] = BitConverter.ToUInt16(byteArray, dataIndex);
                            break;
                        case 3:
                            measureDataPerSensor[sensorIndex, measureIndex] = ConvertToUInt24(byteArray, dataIndex);
                            break;
                        case 4:
                            measureDataPerSensor[sensorIndex, measureIndex] = BitConverter.ToUInt32(byteArray, dataIndex);
                            break;
                        default:
                            return null;
                    }

                    dataIndex += dynamic;
                }
            }

            return measureDataPerSensor;
        }

        public UInt32 ConvertToUInt24(Byte[] rawData, int index)
        {
            if (rawData == null)
                throw new ArgumentNullException("rawData is null", "rawData");
            if (index < 0 || index >= rawData.Length)
                throw new ArgumentOutOfRangeException("index must be positive and lower than array length", "index");
            if (index + 3 > rawData.Length)
                throw new ArgumentException("startIndex equals the length of value minus 2", "index");

            UInt32 value = 0;

            value |= (UInt32)(rawData[index++] & 0xFF);
            value |= (UInt32)((rawData[index++] << 8) & 0xFF00);
            value |= (UInt32)((rawData[index++] << 16) & 0xFF0000);

            return value;
        }



        private DateTime GetTime(byte[] Message)
        {
            UInt32 TimeStamp = BitConverter.ToUInt32(Message, 0);
            System.DateTime dtDateTime = new DateTime(1970, 1, 1, 0, 0, 0, 0, System.DateTimeKind.Local);
            dtDateTime = dtDateTime.AddSeconds(TimeStamp).ToLocalTime();
            return dtDateTime;
        }
        public DateTime CurrentStreaming_Date { get; set; }

        //public string ExtractMeasurement()
        //{
        //    #region Calcdata
        //    var measureNumber = DataLength;
        //    measureNumber /= 3 * this.numberOfActivSensors;
        //    calculatedPayDataLen = (measureNumber * dynamic * this.numberOfActivSensors);
        //    if (calculatedPayDataLen != DataLength)
        //        return null;
        //    if (DYNAMIC_HEADER_LENGTH == 0)
        //    {
        //        return null;
        //    }
        //    this.measureDataPerSensor =
        //        TransformStreamMeasureDataBlock(
        //            this.Message,
        //            DYNAMIC_HEADER_LENGTH,
        //            this.numberOfActivSensors,
        //            NumberOfDaqPerChannel,
        //            dynamic);
        //    if (this.measureDataPerSensor == null)
        //        return "";
        //    #endregion
        //    var data = new StringBuilder();
        //    long baseindex = this.segmentNumber * this.NumberOfDaqPerChannel;

        //    var sensData = new List<List<double>>();

        //    for (int sensorIndex = 0; sensorIndex < this.sensIdList.Count; sensorIndex++)
        //    {
        //        var sens = new List<double>();
        //        for (int dataIndex = 0; dataIndex < NumberOfDaqPerChannel; dataIndex++)
        //        {
        //            var mes = BeanAirConverter.ConvertRawToCalculated(measureDataPerSensor[sensorIndex, dataIndex]);
        //            sens.Add(mes);
        //        }
        //        sensData.Add(sens);
        //    }


        //    for (int j = 0; j < NumberOfDaqPerChannel; j++)
        //    {
        //        long index = baseindex + j;
        //        var line = index.ToString();
        //        for (int i = 0; i < this.numberOfActivSensors; i++)
        //            line += ";" + (sensData[i])[j];
        //        data.AppendLine(line);
        //    }
        //    return data.ToString();
        //}

        public void ExtractMeasurement2(out List<List<double>> _Measures, out List<DateTime> _Dates, out List<long> _Index)
        {
            _Measures = new List<List<double>>();
            _Dates = new List<DateTime>();
            _Index = new List<long>();
            try
            {
                #region Calcdata
                var measureNumber = DataLength;
                measureNumber /= 3 * this.numberOfActivSensors;
                calculatedPayDataLen = (measureNumber * dynamic * this.numberOfActivSensors);
                if (calculatedPayDataLen != DataLength)
                    return;
                if (DYNAMIC_HEADER_LENGTH == 0)
                {
                    return;
                }
                this.measureDataPerSensor =
                    TransformStreamMeasureDataBlock(
                        this.Message,
                        DYNAMIC_HEADER_LENGTH,
                        this.numberOfActivSensors,
                        NumberOfDaqPerChannel,
                        dynamic);
                if (this.measureDataPerSensor == null)
                    return;
                #endregion
                int last_NumberOfDaqPerChannel = this.NumberOfDaqPerChannel;
                if (last_NumberOfDaqPerChannel < this.NumberOfDaqPerChannel_Prev)
                    last_NumberOfDaqPerChannel = this.NumberOfDaqPerChannel_Prev;

                long baseindex = this.segmentNumber * last_NumberOfDaqPerChannel;
                L("baseindex : " + baseindex);


                for (int sensorIndex = 0; sensorIndex < this.sensIdList.Count; sensorIndex++)
                {
                    var sensormeasureModel = new List<double>();
                    for (int dataIndex = 0; dataIndex < NumberOfDaqPerChannel; dataIndex++)
                    {
                        var measureindex = baseindex + dataIndex;
                        var mesure = BeanAirConverter.ConvertRawToCalculated(measureDataPerSensor[sensorIndex, dataIndex]);
                        sensormeasureModel.Add(mesure);
                        if (sensorIndex == 0)
                        {
                            _Index.Add(measureindex);
                            _Dates.Add(CalcDate(measureindex));
                        }

                    }
                    _Measures.Add(sensormeasureModel);
                }

            }
            catch (Exception)
            {

            }
            //_Measures = new List<List<double>>();
            //_Dates = new List<DateTime>();
            //dataPart.Update(this.streamInfoNewComingDate, this.sensIdList, _Measures, _Dates);
            L(string.Join("-", _Index));
        }

        public DateTime CalcDate(double measureIndex)
        {
            DateTime refdate = this.streamInfoNewComingDate;
            try
            {
                ushort SamplingRate = this.samplingFrequency;
                if (SamplingRate > 0 && measureIndex > 0)
                {
                    var microsecond = ((measureIndex / (double)SamplingRate) * 1000000);
                    long TicksPerMicroSecond = 10;//==>  TimeSpan.TicksPerMillisecond / 1000;
                    var ticks = (long)(microsecond * TicksPerMicroSecond);
                    var dt = refdate.AddTicks(ticks);
                    return dt;
                }
            }
            catch (Exception ee)
            {
                var msg = ee.Message;
            }
            return refdate;
        }
    }
}