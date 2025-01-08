using System.Collections.Concurrent;
using System.Net;
using System.Net.Sockets;

namespace SYN6288.App.Communication
{
    internal class TcpClientState
    {
        public Socket Remote { get; set; }

        public byte[] Buffer { get; set; }
    }

    public class TcpServer
    {
        Socket serverSocket = null;
        protected readonly ConcurrentDictionary<string, Socket> clients = new ConcurrentDictionary<string, Socket>();
        public TcpServer()
        {

        }

        public event Action<string, int> Connected;

        public event Action<string, int> Disconnected;

        public event Action<string, int, byte[]> ReceivedData;

        public event Action<Exception> ThrownException;

        public void Create(string ip, int port)
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

            serverSocket.Bind(new IPEndPoint(IPAddress.Any, port));
            serverSocket.Listen(backlog: 15);

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

                Connected?.Invoke(remoteIp, remotePort);

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
                ThrownException?.Invoke(ex);
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
                ThrownException?.Invoke(ex);
            }
            finally
            {
                DoBeginReceive(remote, remoteIp, remotePort);
            }
        }

        protected virtual void OnReceivedData(string ip, int port, byte[] data)
        {
            //SocketReceivedDataEventArgs args = new SocketReceivedDataEventArgs(ip, port, data);
            ReceivedData?.Invoke(ip, port, data);
        }

        public void Send(byte[] tcpPack)
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
                }
                catch (Exception ex)
                {
                    ThrownException?.Invoke(ex);
                    RemoveClient(remoteIp, remotePort);
                }
            }
        }

        public virtual void RemoveClient(string remoteIp, int remotePort)
        {
            var clientKey = $"{remoteIp}:{remotePort}";
            if (clients.TryRemove(clientKey, out Socket client))
            {

                client.Close();
                client.Dispose();
            }
        }
    }
}
