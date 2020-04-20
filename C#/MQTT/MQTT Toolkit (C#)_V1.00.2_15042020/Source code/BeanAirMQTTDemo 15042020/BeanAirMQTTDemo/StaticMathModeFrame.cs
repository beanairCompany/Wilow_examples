using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace BeanAirMQTTDemo
{
    public class StaticMathModeFrame
    {


        /// <summary>
        /// For streamin/SET/Shockdectection
        /// </summary>
        /// <param name="beanDev"></param>
        /// <param name="Measure_Mode"></param>
        /// <param name="dataIndex"></param>
        /// <param name="incomingMessage"></param>
        public static MathDataModel ExtractMathModeData(
            MathDataModel model, int dataIndex, byte[] incomingMessage)
        {
            model.List_Sensors = new List<Sensor>();
            var sensIdList = new List<byte>();
            var sensorEnabledBitmap = BitConverter.ToUInt32(incomingMessage, dataIndex);
            dataIndex += 4;
            for (byte i = 0; i < 5; i++)
            {
                if ((sensorEnabledBitmap & (1 << i)) == (1 << i))
                    sensIdList.Add(i);
            }
            model.SensIdList = sensIdList;


            //-----Start Date of Math Results--------------------

            DateTime startDate = new BeanDate(incomingMessage, dataIndex).ToDateTime(); /* 7 bytes length */
            dataIndex += BeanDate.Length;
            //--------millisecond
            Int16 startDate_ms = BitConverter.ToInt16(incomingMessage, dataIndex);
            dataIndex += 2;
            startDate = startDate.AddMilliseconds(startDate_ms);
            model.DateStart = startDate;
            //-------End Date of Math Results------------------
            DateTime endDate = new BeanDate(incomingMessage, dataIndex).ToDateTime(); ; /* 7 bytes length */
            dataIndex += BeanDate.Length;
            //--------millisecond
            Int16 endDate_ms = BitConverter.ToInt16(incomingMessage, dataIndex);
            dataIndex += 2;
            endDate = endDate.AddMilliseconds(endDate_ms);
            model.DateEnd = endDate;
            //------MeasureNumberPerSegment------------------
            Int16 measureNumberPerSegment = BitConverter.ToInt16(incomingMessage, dataIndex);
            dataIndex += 2;

            double periodMsPerOne = (double)((double)1000.0 / model.SamplingFrequency);
            for (byte id = 0; id < sensIdList.Count; id++)
            {
                #region fileds
                int minValue_SegmentNumber = 0;
                int minValue_Index_InsideTheSegment = 0;
                uint minValueChannel = 0;
                int maxValue_SegmentNumber = 0;
                int maxValue_Index_InsideTheSegment = 0;
                uint maxValueChannel = 0;
                uint Average = 0;
                DateTime minValueDate = DateTime.MinValue;
                int minValueDate_microsec = 0;
                DateTime maxValueDate = DateTime.MinValue;
                int maxValueDate_microsec = 0;

                #endregion

                #region MIN-VALUE
                //------Index corresponding to the Minimum value Channel i-------------------
                minValue_SegmentNumber = BitConverter.ToInt32(incomingMessage, dataIndex);
                dataIndex += 4;
                minValue_Index_InsideTheSegment = BitConverter.ToInt16(incomingMessage, dataIndex);
                dataIndex += 2;
                int minValuebaseindex = minValue_SegmentNumber * measureNumberPerSegment;
                double Min_measureIndex = minValuebaseindex + minValue_Index_InsideTheSegment;
                //---------------Mlin date  value-----------------
                double periodMs_min = periodMsPerOne * Min_measureIndex;
                var flotting_millisec_value_min = periodMs_min - (int)periodMs_min;//0.5 millisecond
                minValueDate_microsec = (int)(flotting_millisec_value_min * 1000);//500 microsec
                minValueDate = startDate.AddMilliseconds((int)periodMs_min);// to get only millisec and not microsec

                //---------Minimum Value on Channel i ----------------
                byte[] arr_min = new byte[4];
                Array.Copy(incomingMessage, dataIndex, arr_min, 0, 3);
                minValueChannel = BitConverter.ToUInt32(arr_min, 0);

                dataIndex += 3;
                #endregion

                #region MAX-VALUE

                //------Index corresponding to the Maximum value Channel i-------------------

                maxValue_SegmentNumber = BitConverter.ToInt32(incomingMessage, dataIndex);
                dataIndex += 4;
                maxValue_Index_InsideTheSegment = BitConverter.ToInt16(incomingMessage, dataIndex);
                dataIndex += 2;

                int maxValuebaseindex = maxValue_SegmentNumber * measureNumberPerSegment;
                double Max_measureIndex = maxValuebaseindex + maxValue_Index_InsideTheSegment;
                //---------------Max date  value-----------------
                double periodMs_max = periodMsPerOne * Max_measureIndex;
                var flotting_millisec_value_max = periodMs_max - (int)periodMs_max;//0.5 millisecond
                maxValueDate_microsec = (int)(flotting_millisec_value_max * 1000);//500 microsec
                maxValueDate = startDate.AddMilliseconds((int)periodMs_max);// to get only millisec and not microsec
                //---------maximum Value on Channel i ----------------
                byte[] arr_max = new byte[4];
                Array.Copy(incomingMessage, dataIndex, arr_max, 0, 3);
                maxValueChannel = BitConverter.ToUInt32(arr_max, 0);
                dataIndex += 3;
                #endregion

                #region AVG-VALUE
                //------Aerage Channel  i ------------------- 
                byte[] arr_avg = new byte[4];
                Array.Copy(incomingMessage, dataIndex, arr_avg, 0, 3);
                Average = BitConverter.ToUInt32(arr_avg, 0);
                dataIndex += 3;
                #endregion

                #region  Update Sensor Data
                var sensor = new Sensor();
                sensor.SensorId = id;
                sensor.MathModeResult_MinValue = BeanAirConverter.ConvertRawToCalculated(minValueChannel);
                sensor.MathModeResult_MaxValue = BeanAirConverter.ConvertRawToCalculated(maxValueChannel);
                sensor.MathModeResult_Average = BeanAirConverter.ConvertRawToCalculated(Average);
                sensor.MathModeResult_MinValueDate = minValueDate;
                sensor.MathModeResult_MaxValueDate = maxValueDate;
                sensor.MathModeResult_MinValueDate_microsec = minValueDate_microsec;
                sensor.MathModeResult_MaxValueDate_microsec = maxValueDate_microsec;
                model.List_Sensors.Add(sensor);
                #endregion
            }
            SetMathModeResult(model);
            return model;
        }
        /// <summary>
        ///  Streaming/S.E.T/Shockdectection
        /// </summary> 
        public static void SetMathModeResult(MathDataModel model)
        {
            try
            {
                var directory = Settings.SUB_LOG_DIRECTORY;

                if (directory == null || directory.Length < 4) { return; }

                if (!Directory.Exists(directory)) Directory.CreateDirectory(directory);
                var fileName = BuildFileName(model.SensIdList, model.DateStart);
                FileInfo fi = new FileInfo(Path.Combine(@directory, @fileName));
                if (fi.Exists == false)
                {
                    using (TextWriter tw = fi.CreateText())
                    {
                        tw.WriteLine(BuildMathResultHeaderFile(model));
                    }
                }
                using (TextWriter tw = fi.AppendText())
                { 
                    try
                    {
                       
                        foreach (var beanSensor in model.List_Sensors)
                        {
                            tw.WriteLine(beanSensor.MathModeResult_Str);
                        }
                    }
                    catch (Exception ex)
                    {
                        var msg = ex.Message;
                    }
                }
            }
            catch (Exception ex)
            {
                var msg = ex.Message;
            }
        }
        private static string BuildFileName(List<byte> SensIdList,DateTime dateStart)
        {
            try
            { 
                var dateStr = dateStart.ToString("yyy_MM_dd_HH:mm:ss");
                var fileName = string.Concat("Transmit_MathResult", "_", dateStr, ".txt");
                fileName = fileName.Replace(" ", "_");
                fileName = fileName.Replace(":", "_");
                fileName = fileName.Replace("/", "_");
                return fileName;
            }
            catch (Exception)
            {
                return "Transmit_Math_NA";
            }
        }
        private static string BuildMathResultHeaderFile(MathDataModel model)
        {
            var str = new StringBuilder();
            try
            {
                str.AppendLine("----------------------------------------------------------------------------------------------");
                str.AppendLine("Topic : " + model.Topic);
                str.AppendLine("Duration : " + model.AcqDuration + " sec");
                str.AppendLine("Cycle : " + model.MeasureCycle_Notif + " sec");
                str.AppendLine(string.Format("Date_start :{0}", model.DateStart.ToString()));
                str.AppendLine(string.Format("Date_end :{0}", model.DateEnd.ToString()));
                str.AppendLine("----------------------------------------------------------------------------------------------");
            }
            catch (Exception ex)
            {
                L("Exp :" + ex.Message);
            }
            return str.ToString();
        }
        private static void L(string v)
        {
            Trace.TraceInformation(v);
        }
    }
}
