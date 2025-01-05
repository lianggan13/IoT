using System.IO.Ports;
using System.Runtime.InteropServices;
using System.Text;

namespace TOFSense_F
{
    public class NAPort
    {
        public NAPort()
        {

        }

        // https://learn.microsoft.com/en-us/dotnet/api/system.io.ports.serialport?view=dotnet-plat-ext-7.0
        SerialPort serialPort;
        public void Start()
        {
            // 0x01, 0x06, 0x00, 0x00, 0x22, 0xB8, 0x91, 0x18
            var buffer = new byte[] { 0x01, 0x06, 0x00, 0x00, 0x22, 0xB8 };
            var crc = ComputeCRC(buffer);
            var bytesCRC = BitConverter.GetBytes(crc);


            serialPort = new SerialPort("COM3");
            // configure serial port
            serialPort.BaudRate = 9600; //  115200 921600 ;//9600 // 921600;//9600;
            serialPort.DataBits = 8;
            serialPort.StopBits = StopBits.One;
            serialPort.Parity = Parity.None;

            serialPort.Open();

            Task.Factory.StartNew(() =>
             Sender(),
             TaskCreationOptions.LongRunning);

            Task.Factory.StartNew(() =>
              Receiver(),
              TaskCreationOptions.LongRunning);
        }


        private async void Sender()
        {
            while (true)
            {
                await Task.Delay(2000);

                var read = new NLink_TOFSense_Read_Frame0();
                var bytes = ByteUtil.StructToBytes(read);
                var sumchk = VerifyCheckSum(bytes);
                read.SumCheck = sumchk;
                var buffer = ByteUtil.StructToBytes(read);

                buffer = new byte[] { 0x01, 0x06, 0x00, 0x00, 0x22, 0xB8, 0x91, 0x18 };

                Console.WriteLine($"<< {string.Join(" ", buffer.Select(b => $"0x{b:X2}"))}");
                //string data = "Hello, SerialPort!";
                //byte[] buffer = Encoding.ASCII.GetBytes(data);
                serialPort.Write(buffer, 0, buffer.Length);
            }
        }

        private void Receiver()
        {
            const int packetLength = 16;

            byte[] buffer = new byte[packetLength];

            // 持续读取数据包
            while (serialPort.IsOpen)
            {
                int bytesRead = 0;
                while (bytesRead < packetLength)
                {
                    bytesRead += serialPort.Read(buffer, bytesRead, buffer.Length - bytesRead);
                    if (bytesRead == 0)
                        break;
                    else
                    {
                        //Console.WriteLine($">> {string.Join(" ", buffer.Select(b => $"0x{b:X2}"))}");
                    }
                }

                if ((buffer[0] == 0x57) && VerifyCheckSum(buffer) == buffer.Last())
                {
                    Console.WriteLine($">> {string.Join(" ", buffer.Select(b => $"0x{b:X2}"))}");
                    var frame = ByteUtil.BytesToStuct<NLink_TOFSense_Frame0>(buffer);
                    StringBuilder sb = new StringBuilder();
                    sb.AppendLine($"time(ms):           {frame.Time}");
                    sb.AppendLine($"dis(m):             {UnpackDistance(frame.Distance)}");
                    sb.AppendLine($"dis_status:         {frame.Status}");
                    sb.AppendLine($"signal_strength:    {frame.SignalStreagth}");
                    sb.AppendLine($"range_precision(cm):{frame.RangePrecision}");
                    Console.WriteLine(sb);
                }
            }
        }

        private float UnpackDistance(byte[] distance)
        {
            byte[] bytes = { distance[0], distance[1], distance[2], 0x00 };
            var n2 = BitOpertion.bytesToInt(bytes);
            var result = n2 / 1000.0f;
            return result;
        }

        private byte VerifyCheckSum(byte[] data)
        {
            byte sum = 0;
            for (int i = 0; i < data.Length - 1; i++)
            {
                sum += data[i];
            }

            return sum;
        }

        public ushort ComputeCRC(byte[] data)
        {
            ushort crc = 0xFFFF;

            for (int i = 0; i < data.Length; i++)
            {
                crc ^= data[i];

                for (int j = 0; j < 8; j++)
                {
                    if ((crc & 0x0001) != 0)
                    {
                        crc >>= 1;
                        crc ^= 0xA001;
                    }
                    else
                    {
                        crc >>= 1;
                    }
                }
            }

            return crc;
        }

    }

    /// <summary>
    /// NLink_TOFSense_Frame0: 57 00 ff 00 9e 8f 00 00 ad 08 00 00 03 00 06 41
    /// </summary>
    [Serializable]
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
    public class NLink_TOFSense_Frame0
    {
        public byte FrameHeader;

        public byte FrameMask;

        public byte Reserverd;

        public byte Id;

        public uint Time;

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)]
        public byte[] Distance;

        public byte Status;

        public ushort SignalStreagth;

        public byte RangePrecision;

        public byte SumCheck;
    }

    /// <summary>
    /// NLink_TOFSense_Read_Frame0: 57 10 FF FF 00 FF FF 63
    /// </summary>
    [Serializable]
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
    public class NLink_TOFSense_Read_Frame0
    {
        public byte FrameHeader = 0x57;

        public byte FrameMask = 0x10;

        public ushort Reserverd = 0xFFFF;

        public byte Id = 0x00;

        public ushort Reserverd2 = 0xFFFF;

        public byte SumCheck = 0x63;
    }
}
