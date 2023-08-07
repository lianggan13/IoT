using System;
using System.Diagnostics;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;

namespace Raspberry.Client.Services
{
    public class SocketClient
    {
        private Socket socket;
        public event Action<object, byte[]> Received = null;
        public SocketClient()
        {
        }

        public void Connect(string serverIp, int serverPort)
        {
            try
            {
                socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
                //socket.Connect(serverIp, serverPort);
                var endPoint = new IPEndPoint(IPAddress.Parse(serverIp), serverPort);

                socket.BeginConnect(endPoint, ConnectCallback, null);
            }
            catch (Exception ex)
            {
                Debug.Fail(ex.Message);
            }
        }

        private void ConnectCallback(IAsyncResult ar)
        {
            socket.EndConnect(ar);


            Task.Factory.StartNew(() =>
                Receiver(), //ReceiverAsync(),
                TaskCreationOptions.LongRunning);
        }

        public void Disconnect()
        {
            try
            {
                if (socket != null)
                {
                    socket.Shutdown(SocketShutdown.Both);
                    socket.Close();
                    socket.Dispose();
                    socket = null;

                    // disconnected event...
                }
            }
            catch (Exception ex)
            {
                Debug.Fail(ex.Message);
            }

        }

        public void Send(string data)
        {
            //data = $"{data} {DateTime.Now:yyyy MM dd HH:mm:ss}";
            byte[] buffer = Encoding.UTF8.GetBytes($"{data}");
            socket.Send(buffer);
        }

        private void Receiver()
        {
            while (socket != null && socket.Connected)
            {
                int bytesRead = 0;
                byte[] lengthBuf = new byte[4];

                try
                {
                    // 接收当前帧的长度
                    while (lengthBuf.Length < 4)
                    {
                        bytesRead += socket.Receive(lengthBuf, bytesRead, lengthBuf.Length - bytesRead, SocketFlags.None);
                        if (bytesRead == 0)
                        {
                            break;
                        }
                    }

                    if (lengthBuf.Length < 0)
                    {
                        break;
                    }

                    // 解析当前帧的长度
                    int frame_length = 0;
                    if (lengthBuf.Length == 4)
                    {
                        if (BitConverter.IsLittleEndian)
                            Array.Reverse(lengthBuf);

                        frame_length = BitConverter.ToInt32(lengthBuf, 0);
                        Debug.WriteLine($"Frame Length: {frame_length}");
                    }

                    // 接收当前帧的数据
                    bytesRead = 0;
                    byte[] frameBuf = new byte[frame_length];
                    while (frameBuf.Length < frame_length)
                    {
                        //int bytesRead = socket.Receive(readBuffer);
                        bytesRead += socket.Receive(frameBuf, bytesRead, frameBuf.Length - bytesRead, SocketFlags.None);
                        if (bytesRead == 0)
                        {
                            break;
                        }
                    }

                    // 解析当前帧的数据
                    if (frameBuf.Length == frame_length)
                    {
                        Debug.WriteLine($"Frame Data: {frameBuf.Length}");
                        Received?.Invoke(this, frameBuf);
                    }
                }
                catch (Exception ex)
                {
                    Disconnect();
                    Debug.WriteLine(ex.Message);
                }
            }
        }

        private void ReceiverAsync()
        {
            const int BufferSize = 2048 * 512;
            while (socket != null && socket.Connected)
            {
                try
                {
                    if (socket.Poll(int.MaxValue, SelectMode.SelectRead))
                    {
                        if (!(socket != null && socket.Connected))
                            continue;

                        byte[] buffer = new byte[BufferSize];
                        socket.BeginReceive(buffer, 0, BufferSize, SocketFlags.None, ReceiveAsyncCallback, buffer);
                    }
                    else
                    {

                    }
                }
                catch (Exception ex)
                {
                    Disconnect();
                    Debug.WriteLine(ex.Message);
                }
            }
        }

        private void ReceiveAsyncCallback(IAsyncResult result)
        {
            int bytesRead = socket.EndReceive(result);
            if (bytesRead > 0)
            {
                byte[] data = result.AsyncState as byte[];
                if (bytesRead > 0)
                {
                    Debug.WriteLine($"Read: {bytesRead}");
                    Received?.Invoke(this, data.Take(bytesRead).ToArray());
                }
            }
        }
    }
}
