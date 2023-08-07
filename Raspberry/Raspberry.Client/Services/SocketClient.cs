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
            //socket = (Socket)result.AsyncState;
            socket.EndConnect(ar);

            Task.Factory.StartNew(() =>
                Receiver(),
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
        const int BufferSize = 1024 * 1024 * 512;

        private void Receiver()
        {
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
