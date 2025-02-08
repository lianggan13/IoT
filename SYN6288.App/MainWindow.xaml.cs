using Amqp;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using SYN6288.App.Communication;
using SYN6288.App.Model;
using System.ComponentModel;
using System.IO.Ports;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Text.RegularExpressions;
using System.Windows;
using System.Windows.Controls;

namespace SYN6288.App
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window, INotifyPropertyChanged
    {
        #region NotifyPropertyChanged
        public event PropertyChangedEventHandler PropertyChanged;

        /// <summary>
        /// 属性改变时触发
        /// </summary>
        /// <param name="propertyName"></param>
        public virtual void OnPropertyChanged([System.Runtime.CompilerServices.CallerMemberName] string propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }


        /// <summary>
        /// 设置需通知的属性的值
        /// </summary>
        /// <typeparam name="T">属性的类型</typeparam>
        /// <param name="item">属性</param>
        /// <param name="value">值</param>
        /// <param name="propertyName">属性名称</param>
        protected virtual void SetProperty<T>(ref T item, T value, [System.Runtime.CompilerServices.CallerMemberName] string propertyName = null)
        {
            if (!EqualityComparer<T>.Default.Equals(item, value))
            {
                item = value;
                OnPropertyChanged(propertyName);
            }
        }

        #endregion

        const int TX_PLOAD_WIDTH = 32; // 32字节有效数据宽度(Len + Data)
        const int RX_PLOAD_WIDTH = 32; // 32字节有效数据宽度(Len + Data)

        private string sendText = "";

        public string SendText
        {
            get { return sendText; }
            set { SetProperty(ref sendText, value); }
        }

        private bool isPortOpen;
        public bool IsPortOpen
        {
            get { return isPortOpen; }
            set
            {
                SetProperty(ref isPortOpen, value);
                if (isPortOpen)
                    serial.Open((string)cmbPorts.SelectedItem);
                else
                    serial.Close();
            }
        }

        private Serial serial;
        private TcpServer tcp;
        private AliYun aliyun;

        private List<byte[]> blocks = new List<byte[]>();

        public AliYunDevice Device { get; set; } = new AliYunDevice();

        public MainWindow()
        {
            InitializeComponent();
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            foreach (string port in SerialPort.GetPortNames().OrderBy(p => p))
                cmbPorts.Items.Add(port);

            serial = new Serial();
            serial.Received += Serial_Received;

            tcp = new TcpServer();
            tcp.Connected += Tcp_Connected;
            tcp.Disconnected += Tcp_Disconnected;
            tcp.ReceivedData += Tcp_ReceivedData;
            tcp.ThrownException += Tcp_ThrownException;

            aliyun = new AliYun();
            aliyun.Received += Aliyun_Received;
            aliyun.ThrownException += Aliyun_ThrownException;
        }

        private void Tcp_ThrownException(Exception ex)
        {
            AddInfo($"{ex.Message}\r\n{ex}");
        }

        private void Aliyun_ThrownException(Exception ex)
        {
            AddInfo($"{ex.Message}\r\n{ex}");
        }

        private void Serial_Received(byte[] buffer)
        {
            var recvText = Serial.GBK.GetString(buffer);
            AddInfo($">> {recvText} {string.Join(" ", buffer.Select(b => $"0x{b:X2}"))}");

            var oldBlock = blocks.FirstOrDefault(b => b.SequenceEqual(buffer)
                                                    || !b.Except(buffer).Any());
            if (oldBlock != null)
                blocks.Remove(oldBlock);

            var newBlock = blocks.FirstOrDefault();
            if (newBlock != null)
                SerialSend(newBlock);
        }

        private void BtnSend_Clicked(object sender, RoutedEventArgs e)
        {
            var txt = SendText;

            var m = int.Parse(numBakVol.Text);
            var v = int.Parse(numForeVol.Text);
            var t = int.Parse(numSpeed.Text);
            var o = !(bool)togStyle.IsChecked ? 0 : 1;

            var prefix = $"[v{v}][m{m}][t{t}][o{o}][n1]"; //  "[v7][m1][t5][o0]"
            txt = $"{prefix}{txt}";

            FillBlock(txt);

            if (radioSerial.IsChecked == true && IsPortOpen)
            {
                // Serial
                var block1 = blocks.First();
                SerialSend(block1);
            }

            if (radioTCP.IsChecked == true)
            {
                // TCP Socket
                TcpSend();
            }

            if (radioAliyun.IsChecked == true)
            {
                // AliYun (MQTT)
                var voice = new
                {
                    voice = txt
                };
                AliYunSend(JsonConvert.SerializeObject(voice).ToLower());
            }
        }

        private void FillBlock(string txt)
        {
            // 选择背景音乐2 (0：无背景音乐  1-15：背景音乐可选)
            // m[0~16]:0背景音乐为静音，16背景音乐音量最大
            // v[0~16]:0朗读音量为静音，16朗读音量最大
            // t[0~5]:0朗读语速最慢，5朗读语速最快
            // o[0~1]:0自然朗读方式，1设置 Word-By-Word 方式
            // n[0~2]:0自动判断，1数字作号码处理，2数字作数值处理
            // [2]:控制标记后的2个汉字强制读成“两字词”
            // [3]:控制标记后的3个汉字强制读成“三字词”

            var buffer = Serial.GBK.GetBytes(txt);

            var len = buffer.Length;
            var DATA_WIDTH = TX_PLOAD_WIDTH - 1;
            var n = len / DATA_WIDTH;

            ClearBlock();

            byte cmd = 0x01; // 0x01 语音合成播放命令
            byte enc = 0x01;
            ushort back = ushort.Parse(numBack.Text);
            byte para = (byte)(enc | (back << 4));

            blocks.Add(new byte[] { cmd, para }); // 命令、参数

            for (int i = 0; i <= n; i++)
            {
                var block = buffer.Skip(i * DATA_WIDTH).Take(DATA_WIDTH).ToArray();
                if (block.Length == 0)
                    continue;

                blocks.Add(block);
            }
        }

        public void SerialSend(byte[] buffer)
        {
            var str = Serial.GBK.GetString(buffer);
            AddInfo($"<< {str} {string.Join(" ", buffer.Select(b => $"0x{b:X2}"))}");

            serial.Send(buffer);
        }

        private void TcpSend()
        {
            var tcpPack = new List<byte>(blocks.SelectMany(b => b)).ToArray();
            tcp.Send(tcpPack);

            AddInfo($"<< {SendText} {string.Join(" ", tcpPack.Select(b => $"0x{b:X2}"))}");
        }

        private void AddInfo(string text)
        {
            text = $"[{DateTime.Now:HH:mm:ss fff}] {text}";
            Application.Current.Dispatcher.Invoke(() =>
            {
                string[] lines = System.Text.RegularExpressions.Regex.Split(tbInfo.Text, Environment.NewLine);
                int count = lines.Length;//count 即为实际的行数
                if (count > 1024)
                {
                    tbInfo.Clear();
                }

                tbInfo.AppendText(text + Environment.NewLine);
                tbInfo.ScrollToEnd();
            });
        }

        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            serial.Close();
        }


        private void BtnStop_Clicked(object sender, RoutedEventArgs e)
        {
            SendCommand(0x02);  // 0x02 停止合成命令
        }

        private void BtnPause_Clicked(object sender, RoutedEventArgs e)
        {
            SendCommand(0x03);  // 0x03 暂停合成命令
        }

        private void BtnResume_Clicked(object sender, RoutedEventArgs e)
        {
            SendCommand(0x04);  // 0x04 恢复合成命令
        }

        private void BtnSleep_Clicked(object sender, RoutedEventArgs e)
        {
            SendCommand(0x88);  // 0x88 芯片进入 Power Down 模式命令
        }

        private void BtnQuery_Clicked(object sender, RoutedEventArgs e)
        {
            SendCommand(0x21);  // 0x21 芯片状态查询命令
        }

        private void SendCommand(byte cmd)
        {
            ClearBlock();

            blocks.Add(new byte[] { cmd });

            var block1 = blocks.First();
            SerialSend(block1);
        }

        private void ClearBlock()
        {
            blocks.Clear();
        }

        private async void btn_SetClicked(object sender, RoutedEventArgs e)
        {
            var btn = (Button)sender;
            btn.IsEnabled = false;
            var cmds = new List<string>()
            {
                txtWifi.Text,
                txtTcp.Text,
            };

            var ip = "192.168.4.1";
            var port = 8080;

            var task = Task.Run(() =>
             {

                 var client = new TcpClient();
                 client.SendTimeout = 13000;
                 client.ReceiveTimeout = 13000;

                 for (int i = 0; i <= 5; i++)
                 {
                     try
                     {
                         client.Connect(IPAddress.Parse(ip), port);
                         if (client.Connected)
                             break;
                     }
                     catch (Exception ex)
                     {
                         AddInfo($"{ex.Message}");
                         if (i == 5)
                             throw ex;
                     }
                 }

                 var stream = client.GetStream();

                 Action<string> send = (message) =>
                 {
                     var data = Encoding.ASCII.GetBytes(message);
                     stream.Write(data, 0, data.Length);

                     AddInfo($"<< {message} [{data.Length}] {string.Join(" ", data.Select(b => $"0x{b:X2}"))}");
                 };

                 Func<string> recv = () =>
                 {
                     byte[] buffer = new byte[128];
                     var bytesRead = stream.Read(buffer, 0, buffer.Length);
                     var data = buffer.Take(bytesRead);
                     var response = Encoding.ASCII.GetString(buffer, 0, bytesRead);

                     AddInfo($">> {response} [{bytesRead}] {string.Join(" ", data.Select(b => $"0x{b:X2}"))}");
                     return response;
                 };


                 const string OK = "OK";
                 foreach (var cmd in cmds)
                 {
                     send.Invoke(cmd);
                     //Thread.Sleep(1500);
                     if (recv.Invoke() == OK)
                         continue;
                 }

                 client.Dispose();
             });


            var error = task.ContinueWith(t =>
            {
                AddInfo($"Set esp8266 [{ip}:{port}] error: \r\n {t.Exception}");
            }, TaskContinuationOptions.OnlyOnFaulted);


            var ok = task.ContinueWith(t =>
            {
                AddInfo($"Set esp8266 [{ip}:{port}] ok.");
            }, TaskContinuationOptions.NotOnFaulted);

            await Task.WhenAny(ok, error);

            btn.IsEnabled = true;
        }

        private void btn_CreateTCPClicked(object sender, RoutedEventArgs e)
        {
            var pattern = @"\""(?<ip>\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})\"",(?<port>\d{1,5})";

            var regex = new Regex(pattern);
            var match = regex.Match(txtTcp.Text);

            var ip = match.Groups["ip"].Value;
            var port = int.Parse(match.Groups["port"].Value);

            tcp.Create(ip, port);

            AddInfo($"Create tcp server [{ip}:{port}] ok.");
        }

        private void Tcp_Connected(string ip, int port)
        {
            AddInfo($"{ip}:{port} connected.");
        }
        private void Tcp_Disconnected(string ip, int port)
        {
            AddInfo($"{ip}:{port} disconnected!");
        }

        private void Tcp_ReceivedData(string ip, int port, byte[] data)
        {
            AddInfo($">> {ip}:{port} {string.Join(" ", data.Select(b => $"0x{b:X2}"))}");
        }

        private void btn_StartAMQP(object sender, RoutedEventArgs e)
        {
            aliyun.StartAMQP();
            AddInfo($">> Start AMQP.");

            var response = aliyun.QueryDevicePropRequest();
            var json = JsonConvert.SerializeObject(response, Formatting.Indented);
            AddInfo($">> {json}");

            var props = response.Body.Data.List.PropertyStatusInfo;
            var led = props.SingleOrDefault(p => p.Identifier == "led");
            var humi = props.SingleOrDefault(p => p.Identifier == "humi");
            var temp = props.SingleOrDefault(p => p.Identifier == "temp");

            Device.Led = int.Parse(led.Value);
            Device.Humi = int.Parse(humi.Value);
            Device.Temp = int.Parse(temp.Value);
        }

        private void btn_SetDevice(object sender, RoutedEventArgs e)
        {
            //var json = JsonConvert.SerializeObject(Device).ToLower();
            // {"led":0,"humi":3,"temp":13}
            var device = new
            {
                led = Device.Led,
            };

            AliYunSend(JsonConvert.SerializeObject(device).ToLower());
        }

        private void AliYunSend(string json)
        {
            AddInfo($"<< {json}");

            var response = aliyun.PubSetDeviceRequest(json);

            AddInfo($">> {JsonConvert.SerializeObject(response)}");
        }

        private void Aliyun_Received(Message message)
        {
            var messageId = message.ApplicationProperties["messageId"];
            var topic = message.ApplicationProperties["topic"].ToString();
            var body = Encoding.UTF8.GetString((byte[])message.Body);

            AddInfo($">> Id:{messageId} Topic:{topic} Body:{body}");

            const string prop_post_topic = "/k21juUUmcsq/stm32/thing/event/property/post";
            if (topic == prop_post_topic)
            {
                var jObject = JObject.Parse(body);
                //var deviceName = jObject["deviceName"].ToObject<string>();
                var items = jObject["items"].ToObject<JObject>();

                if (items.TryGetValue("device", StringComparison.OrdinalIgnoreCase, out var led))
                {
                    Device.Led = led["value"].ToObject<int>();
                }

                if (items.TryGetValue("humi", StringComparison.OrdinalIgnoreCase, out var humi))
                {
                    Device.Humi = humi["value"].ToObject<int>();
                }

                if (items.TryGetValue("temp", StringComparison.OrdinalIgnoreCase, out var temp))
                {
                    Device.Temp = temp["value"].ToObject<int>();
                }
            }
        }
    }
}