using System.ComponentModel;
using System.IO.Ports;
using System.Windows;

namespace NRF24L01.App
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

        const string Start = "：";
        const string End = "。";

        private string sendText = "";

        public string SendText
        {
            get { return sendText; }
            set
            {
                if (!value.StartsWith(Start))
                    value = Start + value;
                if (!value.EndsWith(End))
                    value += End;
                SetProperty(ref sendText, value);
            }
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

        private NAPort serial;
        private List<byte[]> blocks = new List<byte[]>();

        public MainWindow()
        {
            InitializeComponent();
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            foreach (string port in SerialPort.GetPortNames())
                cmbPorts.Items.Add(port);

            serial = new NAPort();
            serial.Received += Serial_Received;
        }

        private void Serial_Received(byte[] buffer)
        {
            var recvText = NAPort.GBK.GetString(buffer);
            AddInfo($">> {recvText} {string.Join(" ", buffer.Select(b => $"0x{b:X2}"))}");

            var oldBlock = blocks.FirstOrDefault(b => b.SequenceEqual(buffer));
            if (oldBlock != null)
                blocks.Remove(oldBlock);

            var newBlock = blocks.FirstOrDefault();
            if (newBlock != null)
                Send(newBlock);
        }

        private void BtnSend_Clicked(object sender, RoutedEventArgs e)
        {
            var txt = SendText.Replace(Start, "").Replace(End, "");

            // 选择背景音乐2 (0：无背景音乐  1-15：背景音乐可选)
            // m[0~16]:0背景音乐为静音，16背景音乐音量最大
            // v[0~16]:0朗读音量为静音，16朗读音量最大
            // t[0~5]:0朗读语速最慢，5朗读语速最快
            // o[0~1]:0自然朗读方式，1设置 Word-By-Word 方式
            // n[0~2]:0自动判断，1数字作号码处理，2数字作数值处理
            // [2]:控制标记后的2个汉字强制读成“两字词”
            // [3]:控制标记后的3个汉字强制读成“三字词”

            var m = int.Parse(numForeVol.Text);
            var v = int.Parse(numBakVol.Text);
            var t = int.Parse(numSpeed.Text);
            var o = !(bool)togStyle.IsChecked ? 0 : 1;

            var prefix = $"[v{v}][m{m}][t{t}][o{o}][n1]"; //  "[v7][m1][t5][o0]"
            txt = $"{prefix}{txt}";
            var buffer = NAPort.GBK.GetBytes(txt);

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

            MarkBlock();

            var block1 = blocks.First();
            Send(block1);
        }

        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            serial.Close();
        }

        public void Send(byte[] buffer)
        {

            var str = NAPort.GBK.GetString(buffer);
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

            MarkBlock();

            var block1 = blocks.First();
            Send(block1);
        }

        private void MarkBlock()
        {
            blocks.Insert(0, (NAPort.GBK.GetBytes(Start)));
            blocks.Add(NAPort.GBK.GetBytes(End));
        }

        private void ClearBlock()
        {
            blocks.Clear();
        }
    }
}