using System.Collections.Concurrent;
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

        private NRF_Serial serial;
        private List<byte[]> blocks = new List<byte[]>();

        public MainWindow()
        {
            Class2.Test2();
            MainClass.Test();
            InitializeComponent();
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            foreach (string port in SerialPort.GetPortNames().OrderBy(p => p))
                cmbPorts.Items.Add(port);

            serial = new NRF_Serial();
            serial.Received += Serial_Received;
        }

        private void Serial_Received(byte[] buffer)
        {
            var recvText = NRF_Serial.GBK.GetString(buffer);
            AddInfo($">> {recvText} {string.Join(" ", buffer.Select(b => $"0x{b:X2}"))}");

            var oldBlock = blocks.FirstOrDefault(b => b.SequenceEqual(buffer)
                                                    || !b.Except(buffer).Any());
            if (oldBlock != null)
                blocks.Remove(oldBlock);

            var newBlock = blocks.FirstOrDefault();
            if (newBlock != null)
                Send(newBlock);
        }

        private void BtnSend_Clicked(object sender, RoutedEventArgs e)
        {
            var txt = SendText;

            {

                // 选择背景音乐2 (0：无背景音乐  1-15：背景音乐可选)
                // m[0~16]:0背景音乐为静音，16背景音乐音量最大
                // v[0~16]:0朗读音量为静音，16朗读音量最大
                // t[0~5]:0朗读语速最慢，5朗读语速最快
                // o[0~1]:0自然朗读方式，1设置 Word-By-Word 方式
                // n[0~2]:0自动判断，1数字作号码处理，2数字作数值处理
                // [2]:控制标记后的2个汉字强制读成“两字词”
                // [3]:控制标记后的3个汉字强制读成“三字词”

                var m = int.Parse(numBakVol.Text);
                var v = int.Parse(numForeVol.Text);
                var t = int.Parse(numSpeed.Text);
                var o = !(bool)togStyle.IsChecked ? 0 : 1;

                var prefix = $"[v{v}][m{m}][t{t}][o{o}][n1]"; //  "[v7][m1][t5][o0]"
                txt = $"{prefix}{txt}";
                var buffer = NRF_Serial.GBK.GetBytes(txt);

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

                var block1 = blocks.First();
                if (IsPortOpen)
                {
                    // Serial
                    Send(block1);
                }


                // TCP Socket
                var tcpPack = new List<byte>(blocks.SelectMany(b => b)).ToArray();
                TcpSend(tcpPack);
            }
        }


        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            serial.Close();
        }

        public void Send(byte[] buffer)
        {
            var str = NRF_Serial.GBK.GetString(buffer);
            AddInfo($"<< {str} {string.Join(" ", buffer.Select(b => $"0x{b:X2}"))}");

            serial.Send(buffer);
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

        private void BtnStop_Clicked(object sender, RoutedEventArgs e)
        {
            // 0x31 设置通讯波特率命令(初始波特率为9600bps)

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
            Send(block1);
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

        Socket serverSocket = null;

        protected readonly ConcurrentDictionary<string, Socket> clients = new ConcurrentDictionary<string, Socket>();
        private void btn_CreateTCPClicked(object sender, RoutedEventArgs e)
        {
            if (serverSocket != null)
            {
                //serverSocket.Close();
                //serverSocket.Dispose();
                //serverSocket = null;
                foreach (var remote in clients.Values)
                {
                    var iPEndPoint = (remote.RemoteEndPoint as IPEndPoint);
                    var remoteIp = iPEndPoint.Address.MapToIPv4().ToString();
                    var remotePort = iPEndPoint.Port;

                    RemoveClient(remoteIp, remotePort);
                }
                return;
            }

            serverSocket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
            serverSocket.ReceiveTimeout = 5000; // receive timout 5 seconds
            serverSocket.SendTimeout = 5000; // send timeout 5 seconds 

            var pattern = @"\""(?<ip>\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})\"",(?<port>\d{1,5})";

            Regex regex = new Regex(pattern);
            Match match = regex.Match(txtTcp.Text);

            var ip = match.Groups["ip"].Value;
            var port = int.Parse(match.Groups["port"].Value);

            serverSocket.Bind(new IPEndPoint(IPAddress.Any, port));
            //serverSocket.Bind(new IPEndPoint(IPAddress.Parse(ip), 1918));
            serverSocket.Listen(backlog: 15);

            AddInfo($"Create tcp server [{ip}:{port}] ok.");

            DoBeginAccept();
        }

        private void DoBeginAccept()
        {
            serverSocket.BeginAccept(AcceptCallback, null);
        }


        private void AcceptCallback(IAsyncResult result)
        {
            DoBeginAccept();

            var remote = serverSocket.EndAccept(result);

            if (remote != null)
            {
                var iPEndPoint = (remote.RemoteEndPoint as IPEndPoint);
                var remoteIp = iPEndPoint.Address.MapToIPv4().ToString();
                var remotePort = iPEndPoint.Port;

                var clientKey = $"{remoteIp}:{remotePort}";
                clients[clientKey] = remote;

                AddInfo($"{clientKey} connected.");

                DoBeginReceive(remote, remoteIp, remotePort);
            }
        }


        private void DoBeginReceive(Socket remote, string remoteIp, int remotePort)
        {
            if (remote == null)
                return;

            //var clientKey = $"{remoteIp}:{remotePort}";

            try
            {
                string clientKey = $"{remoteIp}:{remotePort}";
                while (clients.ContainsKey(clientKey))
                {
                    if (remote.Poll(int.MaxValue, SelectMode.SelectRead))
                    {
                        var buffer = new byte[128];
                        var state = new TcpClientState()
                        {
                            Remote = remote,
                            Buffer = buffer
                        };

                        remote.BeginReceive(buffer, 0, buffer.Length, SocketFlags.None, ReceiveAsyncCallback, state);

                        break;
                    }
                    else if (!remote.Connected)
                        RemoveClient(remoteIp, remotePort); // quit while
                    else
                        RemoveClient(remoteIp, remotePort); // quit while
                }
            }
            catch (Exception ex)
            {
                RemoveClient(remoteIp, remotePort);
                AddInfo($"{ex.Message}\r\n{ex}");
            }
        }

        private void ReceiveAsyncCallback(IAsyncResult result)
        {
            Socket remote = null;
            string remoteIp = "Unknown";
            int remotePort = -1;
            try
            {
                var state = result.AsyncState as TcpClientState;
                remote = state?.Remote;
                if (remote == null)
                    return;

                var iPEndPoint = (remote.RemoteEndPoint as IPEndPoint);
                remoteIp = iPEndPoint.Address.MapToIPv4().ToString();
                remotePort = iPEndPoint.Port;

                if (!remote.Connected)
                {
                    RemoveClient(remoteIp, remotePort);
                    return;
                }
                else
                {
                    int bytes = remote.EndReceive(result);

                    if (bytes == 0)
                        RemoveClient(remoteIp, remotePort);
                    else if (bytes > 0)
                        OnReceivedData(remoteIp, remotePort, state.Buffer.Take(bytes).ToArray());
                }
            }
            catch (Exception ex)
            {
                AddInfo($"{ex.Message}\r\n{ex}");
            }
            finally
            {
                DoBeginReceive(remote, remoteIp, remotePort);
            }
        }

        protected virtual void OnReceivedData(string ip, int port, byte[] data)
        {
            //SocketReceivedDataEventArgs args = new SocketReceivedDataEventArgs(ip, port, data);
            //ReceivedData?.Invoke(this, args);
            AddInfo($">> {ip}:{port} {string.Join(" ", data.Select(b => $"0x{b:X2}"))}");
        }

        private void TcpSend(byte[] tcpPack)
        {
            foreach (var remote in clients.Values)
            {
                if (!remote.Connected)
                    continue;

                var iPEndPoint = (remote.RemoteEndPoint as IPEndPoint);
                var remoteIp = iPEndPoint.Address.MapToIPv4().ToString();
                var remotePort = iPEndPoint.Port;

                try
                {
                    var bytes = remote.Send(tcpPack);
                    AddInfo($"<< {SendText} [{bytes}] {string.Join(" ", tcpPack.Select(b => $"0x{b:X2}"))}");
                }
                catch (Exception ex)
                {
                    AddInfo($"{ex.Message}\r\n{ex}");
                    RemoveClient(remoteIp, remotePort);
                }
            }
        }

        public virtual void RemoveClient(string remoteIp, int remotePort)
        {
            var clientKey = $"{remoteIp}:{remotePort}";
            if (clients.TryRemove(clientKey, out Socket client))
            {
                AddInfo($"{clientKey} disconnected!");
                client.Close();
                client.Dispose();
            }
        }
    }

    internal class TcpClientState
    {
        public Socket Remote { get; set; }

        public byte[] Buffer { get; set; }
    }
}