using System.Diagnostics;
using System.Net.Sockets;
using System.Text;

namespace Raspberry.App.Services
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
                Task con = socket.ConnectAsync(serverIp, serverPort);
                _ = Task.WaitAny(new[] { con }, 5000);
                if (!socket.Connected)
                {
                    socket.Close();
                    return;
                }
            }
            catch (SocketException ex)
            {
                Debug.Fail(ex.Message);
            }

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
            const int ReadBufferSize = 1024 * 1024;
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
