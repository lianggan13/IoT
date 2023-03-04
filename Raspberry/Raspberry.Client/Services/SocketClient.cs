using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;

namespace Raspberry.Client.Services
{
    public class SocketClient
    {
        private readonly Socket socket;
        private NetworkStream netStream;
        public event Action<object, byte[]> Received = null;
        public SocketClient()
        {
            socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
        }

        public void Connect(string serverIp, int serverPort)
        {
            try
            {
                socket.Connect(serverIp, serverPort);
            }
            catch (SocketException ex)
            {
                Debug.Fail(ex.Message);
            }

            if (!socket.Connected)
                return;

            netStream = new NetworkStream(socket, ownsSocket: true);
            netStream.ReadTimeout = 5000;

            Task.Factory.StartNew(() =>
                Receiver(),
                TaskCreationOptions.LongRunning);
        }

        public void Send(string data)
        {
            if (netStream?.CanWrite == true)
            {
                //data = $"{data} {DateTime.Now:yyyy MM dd HH:mm:ss}";
                byte[] buffer = Encoding.UTF8.GetBytes($"{data}");
                netStream.Write(buffer, 0, buffer.Length);
                netStream.Flush();
            }
        }

        private async void Receiver()
        {
            const int ReadBufferSize = 1024 * 512;
            byte[] readBuffer = new byte[ReadBufferSize];

            try
            {
                while (netStream.CanRead)
                {
                    var data = new List<byte>();
                    int bytesRead = 0;
                    while (netStream.DataAvailable)
                    {
                        bytesRead = await netStream.ReadAsync(readBuffer, 0, readBuffer.Length);
                        data.AddRange(readBuffer.Take(bytesRead));
                        Debug.WriteLine($"Read: {bytesRead}");
                    }

                    if (bytesRead > 0)
                        Received?.Invoke(this, data.ToArray());
                }
            }
            catch (OperationCanceledException ex)
            {
                Debug.WriteLine(ex.Message);
            }
            finally
            {
                Array.Clear(readBuffer, 0, ReadBufferSize);
            }
        }
    }
}
