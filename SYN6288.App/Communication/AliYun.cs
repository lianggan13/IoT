using AlibabaCloud.SDK.Iot20180120.Models;
using Amqp;
using Amqp.Framing;
using SYN6288.App.Common;
using System.ComponentModel;
using System.Security.Cryptography;
using System.Text;


namespace SYN6288.App.Communication
{
    public class AliYun : INotifyPropertyChanged
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

        private string Uid;
        private string RegionId;

        private string AccessKey;
        private string AccessSecret;
        private string ProductKey;

        private string IotInstanceId;
        private string ConsumerGroupId; // 实例的消息转发 > 服务端订阅 > 消费组列表查看您的消费组ID
        private string DeviceName;

        private string clientId = Environment.MachineName;

        Address address;



        public event Action<Message>? Received;

        public AliYun()
        {
            var jsonObject = AppSettings.Read();
            if (jsonObject.TryGetValue("AliYun", out var aliyun))
            {
                Uid = aliyun[nameof(Uid)]!.ToObject<string>()!;
                RegionId = aliyun[nameof(RegionId)]!.ToObject<string>()!;

                AccessKey = aliyun[nameof(AccessKey)]!.ToObject<string>()!;
                AccessSecret = aliyun[nameof(AccessSecret)]!.ToObject<string>()!;
                ProductKey = aliyun[nameof(ProductKey)]!.ToObject<string>()!;

                IotInstanceId = aliyun[nameof(IotInstanceId)]!.ToObject<string>()!;
                ConsumerGroupId = aliyun[nameof(ConsumerGroupId)]!.ToObject<string>()!;

                DeviceName = aliyun[nameof(DeviceName)]!.ToObject<string>()!;
            }
        }

        #region AMQP

        public void StartAMQP()
        {
            // Create AMQP
            long timestamp = GetCurrentMilliseconds();
            string param = "authId=" + AccessKey + "&timestamp=" + timestamp;
            string userName = clientId + "|authMode=aksign,signMethod=hmacmd5,consumerGroupId=" + ConsumerGroupId
               + ",iotInstanceId=" + IotInstanceId + ",authId=" + AccessKey + ",timestamp=" + timestamp + "|";
            string password = Sign(param, AccessSecret, "HmacMD5");

            DoConnection(userName, password);

            // ManualResetEvent resetEvent = new ManualResetEvent(false);
            // resetEvent.WaitOne();
        }

        void DoConnection(string userName, string password)
        {
            var host = $"{Uid}.iot-amqp.{RegionId}.aliyuncs.com";
            var port = 5671;

            address = new Address(host, port, userName, password);
            var cf = new ConnectionFactory();
            cf.AMQP.IdleTimeout = 120000;
            //cf.AMQP.ContainerId、cf.AMQP.HostName 自定义
            cf.AMQP.ContainerId = "client.1.2";
            cf.AMQP.HostName = "contoso.com";
            cf.AMQP.MaxFrameSize = 8 * 1024;
            var connection = cf.CreateAsync(address).Result;

            connection.AddClosedCallback(ConnClosed);

            DoReceive(connection);
        }

        void DoReceive(Connection connection)
        {
            var session = new Session(connection);
            var receiver = new ReceiverLink(session, "queueName", null);
            receiver.Start(20, (link, message) =>
            {
                // 消息接收 事件处理 回调
                // 注意：此处不要有耗时的逻辑，如果这里要进行业务处理，请另开线程，否则会堵塞消费。如果消费一直延时，会增加消息重发的概率。
                Received?.Invoke(message);

                link.Accept(message); // ACK
            });
        }

        void ConnClosed(IAmqpObject _, Error e)
        {
            Console.WriteLine("ocurr error: " + e);
            Thread.Sleep(5000);

            DoConnection(address.User, address.Password);
        }

        long GetCurrentMilliseconds()
        {
            DateTime dt1970 = new DateTime(1970, 1, 1);
            DateTime current = DateTime.Now;
            return (long)(current - dt1970).TotalMilliseconds;
        }

        string Sign(string param, string accessSecret, string signMethod)
        {
            //signMethod = HmacMD5
            byte[] key = Encoding.UTF8.GetBytes(accessSecret);
            byte[] signContent = Encoding.UTF8.GetBytes(param);
            var hmac = new HMACMD5(key);
            byte[] hashBytes = hmac.ComputeHash(signContent);
            return Convert.ToBase64String(hashBytes);
        }
        #endregion

        #region HTTP API

        public AlibabaCloud.SDK.Iot20180120.Client CreateApiClient()
        {
            var config = new AlibabaCloud.OpenApiClient.Models.Config();
            config.AccessKeyId = AccessKey;
            config.AccessKeySecret = AccessSecret;
            config.RegionId = RegionId;
            return new AlibabaCloud.SDK.Iot20180120.Client(config);
        }

        public QueryDevicePropertyStatusResponse QueryDevicePropRequest()
        {
            var client = CreateApiClient();
            var queryDevicePropertyStatusRequest = new AlibabaCloud.SDK.Iot20180120.Models.QueryDevicePropertyStatusRequest
            {
                IotInstanceId = IotInstanceId,
                ProductKey = ProductKey,
                DeviceName = DeviceName,
            };
            var runtime = new AlibabaCloud.TeaUtil.Models.RuntimeOptions();
            var response = client.QueryDevicePropertyStatusWithOptions(queryDevicePropertyStatusRequest, runtime);

            return response;
        }

        public PubResponse PubUserRequset(string msg)
        {
            var topic = $"/{ProductKey}/{DeviceName}/user/get";

            var client = CreateApiClient();
            var request = new PubRequest
            {
                IotInstanceId = IotInstanceId,
                ProductKey = ProductKey,
                MessageContent = Convert.ToBase64String(Encoding.Default.GetBytes(msg)), // Base64 String
                TopicFullName = topic,
                Qos = 0, // 支持 QoS0、QoS1
            };
            var response = client.Pub(request);
            return response;
        }

        public SetDevicePropertyResponse PubSetDeviceRequest(string items)
        {
            var client = CreateApiClient();
            var setDevicePropertyRequest = new SetDevicePropertyRequest
            {
                IotInstanceId = IotInstanceId,
                ProductKey = ProductKey,
                DeviceName = DeviceName,
                Items = items,
                Qos = 0,
            };
            AlibabaCloud.TeaUtil.Models.RuntimeOptions runtime = new AlibabaCloud.TeaUtil.Models.RuntimeOptions();

            var response = client.SetDevicePropertyWithOptions(setDevicePropertyRequest, runtime);
            return response;
        }

        #endregion
    }
}
