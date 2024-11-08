using System.IO.Ports;
using System.Text;

namespace NRF24L01.App
{
    public class NAPort
    {
        public event Action<byte[]> Received;

        private SerialPort serialPort;

        public static Encoding GBK { get; }

        static NAPort()
        {
            Encoding.RegisterProvider(CodePagesEncodingProvider.Instance);
            GBK = Encoding.GetEncoding("gbk");
        }

        public NAPort()
        {
            // https://learn.microsoft.com/en-us/dotnet/api/system.io.ports.serialport?view=dotnet-plat-ext-7.0
        }

        public void Open(string port)
        {
            serialPort = new SerialPort(port);
            // configure serial port
            serialPort.BaudRate = 9600; //  115200 921600 ;//9600 // 921600;//9600;
            serialPort.DataBits = 8;
            serialPort.StopBits = StopBits.One;
            serialPort.Parity = Parity.None;
            serialPort.DataReceived += new SerialDataReceivedEventHandler(DataReceivedHandler);
            serialPort.ErrorReceived += new SerialErrorReceivedEventHandler(ErrorReceivedHandler);
            if (!serialPort.IsOpen)
                serialPort.Open();
        }


        public void Close()
        {
            if (serialPort?.IsOpen == true)
            {
                serialPort.Close();
                serialPort.Dispose();
            }
        }

        public void Send(byte[] buffer)
        {
            serialPort.Write(buffer, 0, buffer.Length);
        }

        private void DataReceivedHandler(object sender, SerialDataReceivedEventArgs e)
        {
            //string str = serialPort.ReadExisting();

            int bytesToRead = serialPort.BytesToRead;
            byte[] buffer = new byte[bytesToRead];

            int bytesRead = serialPort.Read(buffer, 0, bytesToRead);

            Received?.Invoke(buffer);
        }

        private void ErrorReceivedHandler(object sender, SerialErrorReceivedEventArgs e)
        {
            throw new Exception(e.ToString());
        }
    }
}
