using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;

namespace BeanAirMQTTDemo
{

    public enum MQTTProcessState
    {
        Init,
        Started,
        Processing,
        Completed,
        Stopped,
        Connetion_Success,
        Connetion_Failure
    }
    public class StreamingDataModel
    {
        public List<byte> SensIdList { get; set; }
        public byte NumberOfActivSensors { get; set; }
        public uint MeasureCycle_Notif { get; set; }
        public uint AcqDuration { get; set; }
        public string Topic { get; internal set; }
        public DateTime StreamInfoNewComingDate { get; set; }
        public ushort SamplingFrequency { get; set; }
        public uint SegmentNumber { get; set; }
        public int NumberOfDaqPerChannel { get; set; }
        public byte Lqi_Value { get; set; }
        public byte TimeSynchronization { get; internal set; }
        public bool IsSynchronized { get { return TimeSynchronization == 1; } }
    }

    public class MathDataModel
    {
        public DateTime DateStart { get; internal set; }
        public DateTime DateEnd { get; internal set; }
        public List<Sensor> List_Sensors { get; internal set; }
        public List<byte> SensIdList { get; set; }
        public byte NumberOfActivSensors { get; set; }
        public uint MeasureCycle_Notif { get; set; }
        public uint AcqDuration { get; set; }
        public string Topic { get; internal set; }
        public short SamplingFrequency { get; internal set; }
    }
    public struct BeanDate
    {
        private UInt16 year;
        private byte month;
        private byte day;
        private byte hour;
        private byte minute;
        private byte second;

        public const int Length = 7;

        public BeanDate(Byte[] byteArray, int startIndex)
        {
            if (byteArray == null) throw new ArgumentNullException();
            if (byteArray.Length < startIndex + 7) throw new ArgumentOutOfRangeException();

            this.year = BitConverter.ToUInt16(byteArray, startIndex);
            startIndex += 2;
            this.month = byteArray[startIndex++];
            this.day = byteArray[startIndex++];
            this.hour = byteArray[startIndex++];
            this.minute = byteArray[startIndex++];
            this.second = byteArray[startIndex++];

        }
        /// <summary>
        /// raise exceptions
        /// </summary>
        /// <returns></returns>
        /// <exception cref="ArgumentException"></exception>
        /// <exception cref="ArgumentOutOfRangeException"></exception>
        public DateTime ToDateTime()
        {
            return new DateTime(this.year, this.month, this.day, this.hour, this.minute, this.second);
        }

    }

    public class Sensor
    {
        public byte SensorId { get; set; }
        public double MathModeResult_Average { get; internal set; }
        public DateTime MathModeResult_MaxValueDate { get; internal set; }
        public int MathModeResult_MaxValueDate_microsec { get; internal set; }
        public double MathModeResult_MinValue { get; internal set; }
        public double MathModeResult_MaxValue { get; internal set; }
        public DateTime MathModeResult_MinValueDate { get; internal set; }
        public int MathModeResult_MinValueDate_microsec { get; internal set; }

        public string MathModeResult_Str
        {
            get
            {
                var s = new StringBuilder();

                s.AppendLine(string.Format("--------------- SensorLabel : {0} ------------------", SensLabelStr));
                s.AppendLine(string.Format("MinValueDate:{0}", this.MathModeResult_MinValueDate.ToString()));
                s.AppendLine(string.Format("MinValue :{0}", this.MathModeResult_MinValue));
                s.AppendLine(string.Format("MaxValueDate:{0}", this.MathModeResult_MaxValueDate.ToString()));
                s.AppendLine(string.Format("MaxValue:{0}", this.MathModeResult_MaxValue));
                s.AppendLine(string.Format("Average :{0}", this.MathModeResult_Average));
                return s.ToString();
            }
        } 
        public string SensLabelStr { get { return "CH_" + SensorId; } }





        public string MathModeResult_MinValueDateStr
        {
            get
            {
                return BeanAirConverter.ConvertToString_MICROSECOND(MathModeResult_MinValueDate, MathModeResult_MinValueDate_microsec);
            }
        }
        public string MathModeResult_MaxValueDateStr
        {
            get
            {
                return BeanAirConverter.ConvertToString_MICROSECOND(MathModeResult_MaxValueDate, MathModeResult_MaxValueDate_microsec);
            }
        }
    }

    public class Settings
    {

        private static bool _MQTT_Broker_DnsFlag = false;
        private static int _MQTT_Broker_Port = 1883;
        private static string _MQTT_Broker_Dns = "";
        private static string _MQTT_Broker_IP = "";
        private static string _LOG_DIRECTORY = "";
        private static string _TOPIC = "";

        public static string LOG_DIRECTORY { set { _LOG_DIRECTORY = value; } }
        public static string SUB_LOG_DIRECTORY
        {
            get
            {
                string default_folder = @"C:\log_beanscape";
                try
                {
                    string topic = _TOPIC;
                    topic = topic.Replace(@"\", "_");
                    topic = topic.Replace(@"/", "_");
                    topic = topic.Replace(@":", "_");
                    topic = topic.Replace(@"*", "_");
                    topic = topic.Replace(@"?", "_");
                    topic = topic.Replace(@"<", "_");
                    topic = topic.Replace(@">", "_");
                    topic = topic.Replace(@"|", "_");
                    return string.Concat(_LOG_DIRECTORY, "\\", topic);
                }
                catch (Exception)
                {

                }
                return default_folder;
            }

        }
        public static bool MQTT_Broker_DnsFlag { get { return _MQTT_Broker_DnsFlag; } set { _MQTT_Broker_DnsFlag = value; } }
        public static string MQTT_Broker_Dns { get { return _MQTT_Broker_Dns; } set { _MQTT_Broker_Dns = value; } }
        public static string MQTT_Broker_IP { get { return _MQTT_Broker_IP; } set { _MQTT_Broker_IP = value; } }
        public static int MQTT_Broker_Port { get { return _MQTT_Broker_Port; } set { _MQTT_Broker_Port = value; } }

        public static string TOPIC { get { return _TOPIC; } set { _TOPIC = value; } }
    }

    public class BeanAirConverter
    {
        public static double ConvertRawToCalculated(uint RawMeasure)
        {
            double ouputVal = Convert.ToDouble(RawMeasure);
            var ratio = 1.0;
            var offset = 0.0;
            if ((RawMeasure & 0x800000) == 0x800000)
                ouputVal = (ouputVal - 0x800000) / (-1000.0);
            else
                ouputVal = ouputVal / 1000.0;

            ouputVal = ratio * ouputVal + offset;
            return Math.Round(ouputVal, 3);
        }


        internal static string ConvertToString_MICROSECOND(DateTime date, int microsec)
        {
            return date.ToString() + "." + microsec + "µs";
        }
    }
}