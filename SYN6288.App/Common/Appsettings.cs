using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System.IO;

namespace SYN6288.App.Common
{
    public static class AppSettings
    {
        public const string JsonFile = "appsettings.json";

        public static JObject Read()
        {
            using (var file = File.OpenText(JsonFile))
            {
                using var reader = new JsonTextReader(file);
                var jsonObject = JObject.Load(reader);

                return jsonObject;
            }
        }
    }
}
