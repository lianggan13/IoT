using Amqp;
using Amqp.Framing;
using Amqp.Sasl;
using System.Net.Security;
using System.Security.Cryptography;
using System.Security.Cryptography.X509Certificates;
using System.Text;

namespace SYN6288.App
{
    class MainClass
    {
        //接入域名，请参见AMQP客户端接入说明文档。
        static string uid = ""; // 账号ID
        public static string regionId = "cn-shanghai";
        static string Host = $"{uid}.iot-amqp.{regionId}.aliyuncs.com"; // 
        static int Port = 5671;

        public static string AccessKey = "";// Environment.GetEnvironmentVariable("ALIBABA_CLOUD_ACCESS_KEY_ID");
        public static string AccessSecret = "";

        public static string productKey = "";

        // 登录物联网平台控制台，在对应实例的消息转发 > 服务端订阅 > 消费组列表查看您的消费组ID
        static string consumerGroupId = "";

        // 表示客户端ID，需您自定义，长度不可超过64个字符。建议使用您的AMQP客户端所在服务器UUID、MAC地址、IP等唯一标识。
        //AMQP客户端接入并启动成功后，登录物联网平台控制台，在对应实例的消息转发 > 服务端订阅 > 消费组列表页签，单击消费组对应的查看，消费组详情页面将显示该参数，方便您识别区分不同的客户端
        static string clientId = "";


        //实例ID。您可在物联网平台控制台的实例概览页面，查看当前实例的ID。
        //若有ID值，必须传入该ID值。
        //若无实例概览页面或ID值，传入空值，即iotInstanceId = ""。

        public static string iotInstanceId = "";
        static int Count = 0;
        static int IntervalTime = 10000;

        static Address address;

        public static void Test()
        {
            long timestamp = GetCurrentMilliseconds();
            string param = "authId=" + AccessKey + "&timestamp=" + timestamp;
            string userName = clientId + "|authMode=aksign,signMethod=hmacmd5,consumerGroupId=" + consumerGroupId
               + ",iotInstanceId=" + iotInstanceId + ",authId=" + AccessKey + ",timestamp=" + timestamp + "|";
            string password = doSign(param, AccessSecret, "HmacMD5");

            DoConnectAmqp(userName, password);

            ManualResetEvent resetEvent = new ManualResetEvent(false);
            resetEvent.WaitOne();
        }

        static void DoConnectAmqp(string userName, string password)
        {
            address = new Address(Host, Port, userName, password);
            //创建Connection。
            ConnectionFactory cf = new ConnectionFactory();
            //如果需要，使用本地TLS。
            //cf.SSL.ClientCertificates.Add(GetCert());
            //cf.SSL.RemoteCertificateValidationCallback = ValidateServerCertificate;
            cf.SASL.Profile = SaslProfile.External;
            cf.AMQP.IdleTimeout = 120000;
            //cf.AMQP.ContainerId、cf.AMQP.HostName请自定义。
            cf.AMQP.ContainerId = "client.1.2";
            cf.AMQP.HostName = "contoso.com";
            cf.AMQP.MaxFrameSize = 8 * 1024;
            var connection = cf.CreateAsync(address).Result;

            //Connection Exception已关闭。
            connection.AddClosedCallback(ConnClosed);

            //接收消息。
            DoReceive(connection);
        }

        static void DoReceive(Connection connection)
        {
            //创建Session。
            var session = new Session(connection);

            //创建ReceiverLink并接收消息。
            var receiver = new ReceiverLink(session, "queueName", null);

            receiver.Start(20, (link, message) =>
            {
                object messageId = message.ApplicationProperties["messageId"];
                object topic = message.ApplicationProperties["topic"];
                string body = Encoding.UTF8.GetString((Byte[])message.Body);
                String replyTo = message.Properties.ReplyTo;
                //注意：此处不要有耗时的逻辑，如果这里要进行业务处理，请另开线程，否则会堵塞消费。如果消费一直延时，会增加消息重发的概率。
                Console.WriteLine("receive message, topic=" + topic + ", messageId=" + messageId + ", body=" + body);

                //ACK消息。
                link.Accept(message);

                //SenderLink sender = new SenderLink(session, "Interop.Server-sender-", replyTo);
                //Message response = new Message(body);
                //response.Properties = new Properties() { CorrelationId = message.Properties.MessageId };

                //try
                //{
                //    sender.Send(response);
                //}
                //catch (Exception exception)
                //{
                //    Console.Error.WriteLine("Error waiting for response to be sent: {0} exception {1}",
                //        response.Body, exception.Message);
                //}
                //sender.Close();
            });
        }

        //static private void SendCommand()
        //{
        //    string audience = Fx.Format("{0}/messages/devicebound", HOST);
        //    string resourceUri = Fx.Format("{0}/messages/devicebound", HOST);

        //    string sasToken = GetSharedAccessSignature(SHARED_ACCESS_KEY_NAME, SHARED_ACCESS_KEY, resourceUri, new TimeSpan(1, 0, 0));
        //    bool cbs = PutCbsToken(connection, HOST, sasToken, audience);

        //    if (cbs)
        //    {
        //        string to = Fx.Format("/devices/{0}/messages/devicebound", DEVICE_ID);
        //        string entity = "/messages/devicebound";

        //        SenderLink senderLink = new SenderLink(session, "sender-link", entity);

        //        var messageValue = Encoding.UTF8.GetBytes("i am a command.");
        //        Message message = new Message()
        //        {
        //            BodySection = new Data() { Binary = messageValue }
        //        };
        //        message.Properties = new Properties();
        //        message.Properties.To = to;
        //        message.Properties.MessageId = Guid.NewGuid().ToString();
        //        message.ApplicationProperties = new ApplicationProperties();
        //        message.ApplicationProperties["iothub-ack"] = "full";

        //        senderLink.Send(message);
        //        senderLink.Close();
        //    }
        //}

        //连接发生异常后，进入重连模式。
        //这里只是一个简单重试的示例，您可以采用指数退避方式，来完善异常场景，重连策略。
        static void ConnClosed(IAmqpObject _, Error e)
        {
            Console.WriteLine("ocurr error: " + e);
            if (Count < 3)
            {
                Count += 1;
                Thread.Sleep(IntervalTime * Count);
            }
            else
            {
                Thread.Sleep(120000);
            }

            //重连。
            DoConnectAmqp(address.User, address.Password);
        }

        static X509Certificate GetCert()
        {
            string certPath = Environment.CurrentDirectory + "/root.crt";
            X509Certificate crt = new X509Certificate(certPath);

            return crt;
        }

        static bool ValidateServerCertificate(object sender, X509Certificate certificate, X509Chain chain, SslPolicyErrors sslPolicyErrors)
        {
            return true;
        }

        static long GetCurrentMilliseconds()
        {
            DateTime dt1970 = new DateTime(1970, 1, 1);
            DateTime current = DateTime.Now;
            return (long)(current - dt1970).TotalMilliseconds;
        }

        //签名方法：支持hmacmd5，hmacsha1和hmacsha256。
        static string doSign(string param, string accessSecret, string signMethod)
        {
            //signMethod = HmacMD5
            byte[] key = Encoding.UTF8.GetBytes(accessSecret);
            byte[] signContent = Encoding.UTF8.GetBytes(param);
            var hmac = new HMACMD5(key);
            byte[] hashBytes = hmac.ComputeHash(signContent);
            return Convert.ToBase64String(hashBytes);
        }
    }
}
