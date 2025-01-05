using System.IO.Ports;
using System.Text;

namespace TOFSense_F
{
    public class Digital
    {
        public Digital()
        {

        }

        SerialPort serialPort;
        public void Start()
        {
            serialPort = new SerialPort("COM3");
            serialPort.BaudRate = 9600;
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
            await Task.Delay(2000);

            while (true)
            {
                //str = DateTime.Now.Second.ToString();
                var num = ParseDecimal(4.90m);
                Console.WriteLine($"<< {string.Join(" ", num.Select(b => $"0x{b:X2}"))}");

                serialPort.Write(num, 0, num.Length);

                await Task.Delay(500);
            }


            while (true)
            {
                foreach (var str in new string[] { "----", "8.8.8.8.", "tttt", "5toP" })
                {
                    var bytes = ParseString(str);
                    Console.WriteLine($"<< {string.Join(" ", bytes.Select(b => $"0x{b:X2}"))}");

                    serialPort.Write(bytes, 0, bytes.Length);

                    await Task.Delay(1000);
                }
            }






            while (true)
            {

                //var buffer = new byte[] { 0x01, 0x06, 0x00, 0x00, 0x22, 0xB8 };
                string str = "123.4";
                //str = DateTime.Now.Second.ToString();
                var num = ParseInteger(str);
                Console.WriteLine($"<< {string.Join(" ", num.Select(b => $"0x{b:X2}"))}");
                serialPort.Write(num, 0, num.Length);

                var point = ParsePoint(str);
                Console.WriteLine($"<< {string.Join(" ", point.Select(b => $"0x{b:X2}"))}");
                serialPort.Write(point, 0, point.Length);

                await Task.Delay(500);
            }
        }

        private byte[] ParseInteger(string str)
        {
            // 01 : 数码管屏的站号（RS485 地址）
            byte addr = 0x01;
            // 功能码： 一个字节 0x03、 0x04 读寄存器;0x06 写单个寄存器;0x10 表示写多个寄存器
            byte func = 0x06;
            // 显示寄存器: 0x0000 整数; 0x0004 小数
            ushort reg = 0x0000;
            // 显示数据       2 位有符号整数， 高字节在前。 22 B8 表示显示8888
            // 小数点的个数  (取值范围: 00 00 – 00 03) 1 表示小数点后 1 位数字， 比如32.4 00 00 表示无小数点
            ushort data = ushort.Parse(str.Replace(".", ""));

            var result = new List<byte>() { addr, func };

            var regbytes = BitConverter.GetBytes(reg);
            if (BitConverter.IsLittleEndian)
                Array.Reverse(regbytes);
            result.AddRange(regbytes);

            var databytes = BitConverter.GetBytes(data);
            if (BitConverter.IsLittleEndian)
                Array.Reverse(databytes);
            result.AddRange(databytes);

            var CRC = ComputeCRC(result.ToArray());
            var bytesCRC = BitConverter.GetBytes(CRC);
            result.AddRange(bytesCRC);

            return result.ToArray();
        }

        private byte[] ParsePoint(string str)
        {
            // 01 : 数码管屏的站号（RS485 地址）
            byte addr = 0x01;
            // 功能码： 一个字节 0x03、 0x04 读寄存器;0x06 写单个寄存器;0x10 表示写多个寄存器
            byte func = 0x06;
            // 显示寄存器: 0x0000 整数; 0x0004 小数
            ushort reg = 0x0004;
            // 显示数据       2 位有符号整数， 高字节在前。 22 B8 表示显示8888
            // 小数点的个数  (取值范围: 00 00 – 00 03) 1 表示小数点后 1 位数字， 比如32.4 00 00 表示无小数点
            int p = str.IndexOf(".");

            ushort data = (ushort)(p == -1 ? 0 : str.Length - p - 1);

            var result = new List<byte>() { addr, func };
            var regbytes = BitConverter.GetBytes(reg);
            if (BitConverter.IsLittleEndian)
                Array.Reverse(regbytes);
            result.AddRange(regbytes);

            var databytes = BitConverter.GetBytes(data);
            if (BitConverter.IsLittleEndian)
                Array.Reverse(databytes);
            result.AddRange(databytes);

            var CRC = ComputeCRC(result.ToArray());
            var bytesCRC = BitConverter.GetBytes(CRC);
            result.AddRange(bytesCRC);

            return result.ToArray();
        }

        private byte[] ParseDecimal(decimal num)
        {
            // 01 10 00 90 00 02 04 00 02 01 EA DB 1C
            byte addr = 0x01;
            // 功能码： 一个字节 0x03、 0x04 读寄存器;0x06 写单个寄存器;0x10 表示写多个寄存器
            byte func = 0x10;
            // 显示寄存器: 0x0000 整数; 0x0004 小数
            ushort reg = 0x0090;
            // 寄存器个数
            ushort regn = 0x0002;
            // 数据个数
            byte datan = 0x04;
            // 00 表示正负号（00=正数;01=负数， 数字前显示-）
            byte sign = (byte)(num >= 0 ? 0x00 : 0x01);

            //  02 表示小数点后有2位数字， 0 表示无小数点
            string str = num.ToString(); //("X");
            int p = str.IndexOf(".");
            byte point = (byte)(p == -1 ? 0 : str.Length - p - 1);

            // 2 位整数， 高字节在前。 01 EA 表示十进制 490
            //ushort data = 0x01EA;
            ushort data = ushort.Parse(str.Replace(".", ""));

            var result = new List<byte>() { addr, func };

            var regbytes = BitConverter.GetBytes(reg);
            if (BitConverter.IsLittleEndian)
                Array.Reverse(regbytes);
            result.AddRange(regbytes);

            var regnbytes = BitConverter.GetBytes(regn);
            if (BitConverter.IsLittleEndian)
                Array.Reverse(regnbytes);
            result.AddRange(regnbytes);

            result.Add(datan);

            result.Add(sign);

            result.Add(point);

            var databytes = BitConverter.GetBytes(data);
            if (BitConverter.IsLittleEndian)
                Array.Reverse(databytes);
            result.AddRange(databytes);

            var CRC = ComputeCRC(result.ToArray());
            var bytesCRC = BitConverter.GetBytes(CRC);
            result.AddRange(bytesCRC);

            return result.ToArray();
        }

        private byte[] ParseString(string str)
        {
            // 01 10 00 70 00 03 06 50 32 2E 30 20 20 C0 00
            byte addr = 0x01;
            // 功能码： 一个字节 0x03、 0x04 读寄存器;0x06 写单个寄存器;0x10 表示写多个寄存器
            byte func = 0x10;
            // 显示寄存器: 0x0000 整数; 0x0004 小数
            ushort reg = 0x0070;
            // 寄存器个数
            ushort regn = 0x0003;
            // 数据段的字节数
            byte datan = 0x06;

            // ASCII 字符串
            var data = Encoding.ASCII.GetBytes(str);
            datan = (byte)data.Length;
            regn = (ushort)(data.Length / 2);

            var result = new List<byte>() { addr, func };

            var regbytes = BitConverter.GetBytes(reg);
            if (BitConverter.IsLittleEndian)
                Array.Reverse(regbytes);
            result.AddRange(regbytes);

            var regnbytes = BitConverter.GetBytes(regn);
            if (BitConverter.IsLittleEndian)
                Array.Reverse(regnbytes);
            result.AddRange(regnbytes);

            result.Add(datan);

            result.AddRange(data);

            var CRC = ComputeCRC(result.ToArray());
            var bytesCRC = BitConverter.GetBytes(CRC);
            result.AddRange(bytesCRC);

            return result.ToArray();
        }

        private byte[] ParseLight(ushort data)
        {
            // 01 : 数码管屏的站号（RS485 地址）
            byte addr = 0x01;
            // 功能码： 一个字节 0x03、 0x04 读寄存器;0x06 写单个寄存器;0x10 表示写多个寄存器
            byte func = 0x06;
            // 亮度寄存器
            ushort reg = 0x0005;

            var result = new List<byte>() { addr, func };
            var regbytes = BitConverter.GetBytes(reg);
            if (BitConverter.IsLittleEndian)
                Array.Reverse(regbytes);
            result.AddRange(regbytes);

            // 亮度值: 0x0000 ~ 0x0007
            var databytes = BitConverter.GetBytes(data);
            if (BitConverter.IsLittleEndian)
                Array.Reverse(databytes);
            result.AddRange(databytes);

            var CRC = ComputeCRC(result.ToArray());
            var bytesCRC = BitConverter.GetBytes(CRC);
            result.AddRange(bytesCRC);

            return result.ToArray();
        }

        private void Receiver()
        {
            const int packetLength = 8;

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

                Console.WriteLine($">> {string.Join(" ", buffer.Select(b => $"0x{b:X2}"))}");
            }
        }

        private float UnpackDistance(byte[] distance)
        {
            byte[] bytes = { distance[0], distance[1], distance[2], 0x00 };
            var n2 = BitOpertion.bytesToInt(bytes);
            var result = n2 / 1000.0f;
            return result;
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
}
