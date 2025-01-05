using Newtonsoft.Json;
using System.Text;

using Tea;

namespace SYN6288.App
{
    public class Class2
    {
        /**
         * 使用AK&SK初始化Client。
         * @param accessKeyId 
         * @param accessKeySecret
         * @param regionId
         * @return Client
         * @throws Exception
         */
        public static AlibabaCloud.SDK.Iot20180120.Client CreateClient()
        {
            AlibabaCloud.OpenApiClient.Models.Config config = new AlibabaCloud.OpenApiClient.Models.Config();
            // 您的AccessKey ID。
            config.AccessKeyId = MainClass.AccessKey;
            // 您的AccessKey Secret。
            config.AccessKeySecret = MainClass.AccessSecret;
            // 您的可用区ID。
            config.RegionId = MainClass.regionId;
            return new AlibabaCloud.SDK.Iot20180120.Client(config);
        }


        public static void Test()
        {
            // "/${productKey}/${deviceName}/user/get"
            //var topic = "/k21juUUmcsq/stm32/user/get";
            var topic = "/sys/k21juUUmcsq/stm32/thing/service/property/set";
            var body = new
            {
                method = "thing.service.property.set",
                id = "6666688888",
                @params = new
                {
                    led = 0,
                },
                version = "1.0.0",
            };
            var msg = JsonConvert.SerializeObject(body);
            try
            {
                AlibabaCloud.SDK.Iot20180120.Client client = CreateClient();
                AlibabaCloud.SDK.Iot20180120.Models.PubRequest request = new AlibabaCloud.SDK.Iot20180120.Models.PubRequest
                {
                    // 物联网平台实例ID。
                    IotInstanceId = MainClass.iotInstanceId,
                    // 产品ProductKey。
                    ProductKey = MainClass.productKey,
                    // 要发送的消息主体，hello world Base64 String。
                    MessageContent = Convert.ToBase64String(Encoding.Default.GetBytes(msg)),
                    // 要接收消息的设备的自定义Topic。
                    TopicFullName = topic,
                    // 指定消息的发送方式，支持QoS0和QoS1。
                    Qos = 0,
                };
                AlibabaCloud.SDK.Iot20180120.Models.PubResponse response = client.Pub(request);
                Console.WriteLine("publish message result: " + response.Body.Success);
                Console.WriteLine(response.Body.Code);
                Console.WriteLine(response.Body.ErrorMessage);
            }
            catch (TeaException error)
            {
                Console.WriteLine(error.Code);
                Console.WriteLine(error.Message);
            }
            catch (Exception _error)
            {
                Console.WriteLine(_error.Message);
                Console.WriteLine(_error.StackTrace);
            }
        }


        public static void Test2()
        {
            try
            {
                AlibabaCloud.SDK.Iot20180120.Client client = CreateClient();
                AlibabaCloud.SDK.Iot20180120.Models.SetDevicePropertyRequest setDevicePropertyRequest = new AlibabaCloud.SDK.Iot20180120.Models.SetDevicePropertyRequest
                {
                    IotInstanceId = MainClass.iotInstanceId,
                    ProductKey = MainClass.productKey,
                    DeviceName = "stm32",
                    Items = "{\"led\":0}",
                    Qos = 0,
                };
                AlibabaCloud.TeaUtil.Models.RuntimeOptions runtime = new AlibabaCloud.TeaUtil.Models.RuntimeOptions();

                var response = client.SetDevicePropertyWithOptions(setDevicePropertyRequest, runtime);
                Console.WriteLine("publish message result: " + response.Body.Success);
                Console.WriteLine(response.Body.Code);
                Console.WriteLine(response.Body.ErrorMessage);
            }
            catch (TeaException error)
            {
                // 此处仅做打印展示，请谨慎对待异常处理，在工程项目中切勿直接忽略异常。
                // 错误 message
                Console.WriteLine(error.Message);
                // 诊断地址
                Console.WriteLine(error.Data["Recommend"]);
                AlibabaCloud.TeaUtil.Common.AssertAsString(error.Message);
            }
            catch (Exception _error)
            {
                TeaException error = new TeaException(new Dictionary<string, object>
                {
                    { "message", _error.Message }
                });
                // 此处仅做打印展示，请谨慎对待异常处理，在工程项目中切勿直接忽略异常。
                // 错误 message
                Console.WriteLine(error.Message);
                // 诊断地址
                Console.WriteLine(error.Data["Recommend"]);
                AlibabaCloud.TeaUtil.Common.AssertAsString(error.Message);
            }
        }
    }
}