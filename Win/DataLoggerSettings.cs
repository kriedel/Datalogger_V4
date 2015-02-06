using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;
using System.Threading;
using Hid;
using ZedGraph;

namespace Project1HostApp
{
    public partial class frmDataloggerSettings : Form
    {
        private const int DEVICE_VID = 0x03EB;
        private const int DEVICE_PID = 0x2063;
        private FileInfo[] content;
        DirectoryInfo currentpath;
        LineItem curve0;
        LineItem curve1;
        LineItem curve2;
        LineItem curve3;
        long value, daysleft;
        // data point list
        RollingPointPairList list1 = new RollingPointPairList(120);
        RollingPointPairList list2 = new RollingPointPairList(120);
       
        private IDevice GetDeviceConnection()
        {
            IDevice[] ConnectedDevices = DeviceFactory.Enumerate(DEVICE_VID, DEVICE_PID);
            IDevice ConnectionHandle = null;
            
                if (ConnectedDevices.Count() > 0)
                    ConnectionHandle = ConnectedDevices[0];
                else
                    return null;

                // Fix report handle under Windows
                if (ConnectionHandle is Hid.Win32.Win32DeviceSet)
                {
                    ((Hid.Win32.Win32DeviceSet)ConnectionHandle).AddDevice(0x00,
                        ((Hid.Win32.Win32DeviceSet)ConnectionHandle).UnallocatedDevices[0]);
                }

                return ConnectionHandle;
        }

        public frmDataloggerSettings()
        {
            InitializeComponent();
        }
        
        private byte[] StringToByteArray(string str)
        {
            System.Text.ASCIIEncoding enc = new System.Text.ASCIIEncoding();
            return enc.GetBytes(str);
        }

        private string ByteArrayToString(byte[] arr)
        {
            System.Text.ASCIIEncoding enc = new System.Text.ASCIIEncoding();
            return enc.GetString(arr);
        }

        private void btnSetValues_Click(object sender, EventArgs e)
        {
            IDevice ConnectionHandle = GetDeviceConnection();

            if (ConnectionHandle == null)
            {
                MessageBox.Show("Error: Cannot connect to DataLogger device.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                return;
            }

            
            byte[] Answer = new byte[20];
            //byte[] temp;
            int LogIntervall;

            if (cBTime.Checked)
            {
                dtpDate.Value = DateTime.Now;
                dtpTime.Value = DateTime.Now;
            }

            Answer[0] = (byte)dtpTime.Value.Hour;
            Answer[1] = (byte)dtpTime.Value.Minute;
            Answer[2] = (byte)dtpTime.Value.Second;
            Answer[3] = (byte)dtpDate.Value.Day;
            Answer[4] = (byte)dtpDate.Value.Month; ;
            Answer[5] = (byte)((dtpDate.Value.Year < 2000) ? 0 : (dtpDate.Value.Year - 2000));
            LogIntervall = ((int)nudLogInterval.Value * 2);
            Answer[6] = (byte)LogIntervall;
            Answer[7] = (byte)(LogIntervall >> 8);
            Answer[8] = (byte)(sbyte)nudcorrection1.Value;
            Answer[9] = (byte)(sbyte)nudcorrection2.Value;
            Answer[10] = (byte)(sbyte)nudcorrection3.Value;
            Answer[11] = (byte)(sbyte)nudcorrection4.Value;

            Answer[16] = (byte)daysleft;
            Answer[17] = (byte)(daysleft>>8);

            LogIntervall = ((int)nudWebLogInterval.Value * 2);
            Answer[18] = (byte)LogIntervall;
            Answer[19] = (byte)(LogIntervall >> 8);

            /*temp = StringToByteArray(txtlocation1.Text);
            for (int i = 0; i < temp.Length; i++)
                Answer[i + 13] = temp[i];
            
            temp = StringToByteArray(txtlocation2.Text);
            for (int i = 0; i < temp.Length; i++)
                Answer[i + 21] = temp[i];
            
            temp = StringToByteArray(txtlocation3.Text);
            for (int i = 0; i < temp.Length; i++)
                Answer[i + 29] = temp[i];*/

            try
            {
                ConnectionHandle.Write(0x00, Answer);
                MessageBox.Show("Device parameters updated successfully.", "Success", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
            catch (Exception ex)
            {
                MessageBox.Show("Error: " + ex.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
            }
        }

        private void btnGetValues_Click(object sender, EventArgs e)
        {
            IDevice ConnectionHandle = GetDeviceConnection();

            if (ConnectionHandle == null)
            {
                MessageBox.Show("Error: Cannot connect to DataLogger device.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                return;
            }
            
            byte[] Response = new byte[20];
            byte[] temp = new byte[10];
            int LogIntervall;

            try
            {
                ConnectionHandle.Read(0x00, Response);
                
                try
                {
                    dtpDate.Value = new DateTime(
                         (2000 + Response[5]), Response[4], Response[3]);

                    dtpTime.Value = new DateTime(
                        DateTime.Now.Year, DateTime.Now.Month, DateTime.Now.Day,
                        Response[0], Response[1], Response[2]);
                }
                catch 
                {
                    dtpDate.Value = DateTime.Now;
                    dtpTime.Value = DateTime.Now;
                }

                try
                {
                    LogIntervall = Response[7];
                    LogIntervall <<= 8;
                    LogIntervall += Response[6];
                    nudLogInterval.Value = LogIntervall / 2;
                }
                catch 
                {
                    nudLogInterval.Value = nudLogInterval.Minimum;
                }

                try
                {
                    nudcorrection1.Value = (sbyte)Response[8];
                   // txttemp1.Text = Response[11].ToString() + " °C" +" / "+ Response[12].ToString() + " %";

                    nudcorrection2.Value = (sbyte)Response[9];
                    
                    nudcorrection3.Value = (sbyte)Response[10];

                    nudcorrection4.Value = (sbyte)Response[11];
                   /* for (int i = 0; i < 8; i++) temp[i] = Response[i + 13];
                    txtlocation1.Text = ByteArrayToString(temp);

                    for (int i = 0; i < 8; i++) temp[i] = Response[i + 21];
                    txtlocation2.Text = ByteArrayToString(temp);

                    for (int i = 0; i < 8; i++) temp[i] = Response[i + 29];
                    txtlocation3.Text = ByteArrayToString(temp);*/
                    
                    value = Response[15];
                    value <<= 8;
                    value += Response[14];
                    value <<= 8;
                    value += Response[13];
                    value <<= 8;
                    value += Response[12];
                    value *= 512;               // one sector per cluster, 512 bytes per sector 
                    txtfreemem.Text = value.ToString() + " Bytes"; // free memory on device

                    daysleft = value / ((86400 * 26 / (long)nudLogInterval.Value));
                    txtdayleft.Text = daysleft.ToString() + " Days";
                                     
                }
                catch
                {
                }

                try
                {
                    LogIntervall = Response[19];
                    LogIntervall <<= 8;
                    LogIntervall += Response[18];
                    nudWebLogInterval.Value = LogIntervall / 2;
                }
                catch
                {
                    nudWebLogInterval.Value = nudWebLogInterval.Minimum;
                }


                MessageBox.Show("Device parameters retrieved successfully.", "Success", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
            catch (Exception ex)
            {
                MessageBox.Show("Error: " + ex.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
            }
        }

        private void frmDataloggerSettings_Load(object sender, EventArgs e)
        {
            toolStripStatusLabelDate.Text = "";
            CreateGraph(zedGraphControl1);
        }


        private void btnReadFolder_Click(object sender, EventArgs e)
        {
            if (openFileDialog1.ShowDialog() == DialogResult.OK)
            {
                currentpath = new DirectoryInfo(Path.GetDirectoryName(openFileDialog1.FileName));     //Dateipfad ermitteln
                content = currentpath.GetFiles("*.txt"); //Dateien in Liste einlesen

                dGVFiles.Rows.Clear();

                for (int i = 0; i < content.Length; i++)
                    dGVFiles.Rows.Add(content[i].CreationTime.ToShortDateString(), content[i].Name.ToString()); //Namen auslesen

                try
                {

                }
                catch
                {
                }
            }
        }

        private void CreateGraph(ZedGraphControl zgc)
        {
            // Get a reference to the GraphPane instance in the ZedGraphControl
            GraphPane myPane = zgc.GraphPane;

            // Set the titles and axis labels
            myPane.Title.Text = "Data Logger";
            myPane.XAxis.Title.Text = "Time";
            myPane.YAxis.Title.Text = "Temperature [°C]";
            myPane.Y2Axis.Title.Text = "Humidity [%]";

            // Add gridlines
            myPane.XAxis.MajorGrid.IsVisible = true;
            myPane.YAxis.MajorGrid.IsVisible = true;
           // myPane.Y2Axis.MajorGrid.IsVisible = true;
            myPane.XAxis.MajorGrid.Color = Color.LightBlue;
            myPane.YAxis.MajorGrid.Color = Color.LightBlue;
            //myPane.Y2Axis.MajorGrid.Color = Color.LightBlue;

            // Make the Y axis scale colors
            myPane.YAxis.Scale.FontSpec.FontColor = Color.Blue;
            myPane.YAxis.Title.FontSpec.FontColor = Color.Blue;
            myPane.Y2Axis.Scale.FontSpec.FontColor = Color.DarkGray;
            myPane.Y2Axis.Title.FontSpec.FontColor = Color.DarkGray;

            // Manually set the axis range
            myPane.YAxis.Scale.Min = -20;
            myPane.YAxis.Scale.Max = 60;
            myPane.Y2Axis.Scale.Min = 0;
            myPane.Y2Axis.Scale.Max = 100;

            // First Y Axis Settings
            //myPane.YAxis.MajorGrid.IsZeroLine = false;
            myPane.YAxis.MinorTic.IsOpposite = false;
            myPane.YAxis.MajorTic.IsOpposite = false;

            // Y2 Axis Settings
            myPane.Y2Axis.IsVisible = true;
            myPane.Y2Axis.MinorTic.IsOpposite = false;
            myPane.Y2Axis.MajorTic.IsOpposite = false;

            // Fill the axis background with a gradient
            myPane.Chart.Fill = new Fill(Color.White, Color.LightGoldenrodYellow, 45F);
            //myPane.Chart.Fill = new Fill(Color.White, Color.LightGray, 45.0f);
            myPane.Fill = new Fill(Color.White, Color.FromArgb(220, 220, 255), 45F);
            //myPane.Fill = new Fill(Color.LightSlateGray, Color.White, 45.0f);
            // Legend position
            myPane.Legend.Position = LegendPos.Top;


            // Set the XAxis to date type
            myPane.XAxis.Type = AxisType.Date;
            myPane.XAxis.Scale.MajorUnit = DateUnit.Second;
            zedGraphControl1.GraphPane.XAxis.Scale.Format = "T";
            zedGraphControl1.GraphPane.XAxis.Type = AxisType.Date;

            // x-Axis type, scale in minutes after start
            //myPane.XAxis.Scale.Min = 0;
            //myPane.XAxis.Scale.Max = 20;
            //myPane.XAxis.Scale.MinorStep = 1;
            //myPane.XAxis.Scale.MajorStep = 5;


            // Initially, a curve is added with no data points (list is empty)
            curve0 = myPane.AddCurve("", list1, Color.Blue, SymbolType.None);
            curve1 = myPane.AddCurve("", list2, Color.DarkGray, SymbolType.None);

            // curve line widths
            curve0.Line.Width = 1.8F;
            curve1.Line.Width = 1.8F;
            
            zgc.AxisChange();
            zgc.Invalidate();
         }

        // Set the size and location of the ZedGraphControl
        private void SetSize()
        {
            // Control is always 10 pixels inset from the client rectangle of the form
            Rectangle formRect = this.ClientRectangle;
            formRect.Inflate(-10, -10);

            if (zedGraphControl1.Size != formRect.Size)
            {
                zedGraphControl1.Location = formRect.Location;
                zedGraphControl1.Size = formRect.Size;
            }
        }

        private void frmDataloggerSettings_Resize(object sender, EventArgs e)
        {
            SetSize();
        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            toolStripStatusLabelDate.Text = DateTime.Now.ToLongDateString()+", "+DateTime.Now.ToLongTimeString();
        }

        private void nudLogInterval_ValueChanged(object sender, EventArgs e)
        {
            txtdayleft.Text = (value / (86400 * 26 / (long)nudLogInterval.Value)).ToString() + " Days";
        }

        private void dGVFiles_CellMouseClick(object sender, DataGridViewCellMouseEventArgs e)
        {
            string filename;

            PointPairList list1 = new PointPairList();
            PointPairList list2 = new PointPairList();
            PointPairList list3 = new PointPairList();
            PointPairList list4 = new PointPairList();

            filename = currentpath.ToString() + "\\" + dGVFiles.SelectedCells[1].Value.ToString();

            if (!File.Exists(filename))
            {
                MessageBox.Show(filename + " not found.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                return;
            }
            using (StreamReader sr = new StreamReader(filename))
            {
                string input;
                while ((input = sr.ReadLine()) != null)
                {
                    //row_length = input.Length;
                    string[] output = input.Split(','); // get the single values
                    list1.Add((double)new XDate(Convert.ToDateTime(output[0])),
                        Convert.ToDouble(output[1]));
                    list2.Add((double)new XDate(Convert.ToDateTime(output[0])),
                        Convert.ToDouble(output[2]));
                    list3.Add((double)new XDate(Convert.ToDateTime(output[0])),
                        Convert.ToDouble(output[3]));
                    list4.Add((double)new XDate(Convert.ToDateTime(output[0])),
                        Convert.ToDouble(output[4]));
                }
            }

            btnClear_Click(this, e);
            
            if (cBCurve0.Checked)
            {
                curve0 = zedGraphControl1.GraphPane.AddCurve("", list1, Color.Blue, SymbolType.None);
                curve0.Line.Width = 1.8F;
                curve0.Line.IsSmooth = true;
                curve0.Label.Text = "Sensor 1:" + " Temperature";
                cBCurve0.Text = curve0.Label.Text;
            }
            
            if (cBCurve1.Checked)
            {
                curve1 = zedGraphControl1.GraphPane.AddCurve("", list2, Color.DarkGray, SymbolType.None);
                curve1.Line.Width = 1.8F;
                curve1.Line.IsSmooth = true;
                curve1.IsY2Axis = true;
                curve1.Label.Text = "Sensor 1:" + " Humidity";
                cBCurve1.Text = curve1.Label.Text;
            }

            if (cBCurve2.Checked)
            {
                curve2 = zedGraphControl1.GraphPane.AddCurve("", list3, Color.Red, SymbolType.None);
                curve2.Line.Width = 1.8F;
                curve2.Line.IsSmooth = true;
                curve2.Label.Text = "Sensor 2:";
                cBCurve2.Text = curve2.Label.Text;
            }

            if (cBCurve3.Checked)
            {
                curve3 = zedGraphControl1.GraphPane.AddCurve("", list4, Color.Green, SymbolType.None);
                curve3.Line.Width = 1.8F;
                curve3.Line.IsSmooth = true;
                curve3.Label.Text = "Sensor 3:";
                cBCurve3.Text = curve3.Label.Text;
            }

            zedGraphControl1.AxisChange();
            zedGraphControl1.Invalidate();
        }

               
         private void btnClear_Click(object sender, EventArgs e)
         {
             try
             {
                 curve0.Clear();
                 curve0.Label.Text = "";
             }
             catch { }
             try
             {
                 curve1.Clear();
                 curve1.Label.Text = "";
             }
             catch { }
             try
             {
                 curve2.Clear();
                 curve2.Label.Text = "";
             }
             catch
             {
             }
             try
             {
                 curve3.Clear();
                 curve3.Label.Text = "";
             }
             catch
             {
             }

             zedGraphControl1.AxisChange();
             zedGraphControl1.Invalidate();
         }

      }
}
