#include "mqtt.h"

Mqtt mqtt;

static void trim_crlf(const char **p, size_t *len)
{
    while (*len > 0)
    {
        char c = (*p)[*len - 1];
        if (c == '\r' || c == '\n' || c == '\0')
            (*len)--;
        else
            break;
    }
}

void mqtt_config(int scheme,
                 const char *clientId, const char *username, const char *password,
                 int certKeyId, int caId, const char *path,
                 uint32_t timeoutMs)
{
    if (!mqtt.esp8266)
        return;

    if (!clientId)
        clientId = "";
    if (!username)
        username = "";
    if (!password)
        password = "";
    if (!path)
        path = "";

    char buf[256];
    snprintf(buf, sizeof(buf), "AT+MQTTUSERCFG=%d,%d,\"%s\",\"%s\",\"%s\",%d,%d,\"%s\"",
             mqtt.linkId, scheme, clientId, username, password, certKeyId, caId, path);
    mqtt.esp8266->send(buf, timeoutMs);
}

void mqtt_connect(const char *host, int port, int reconnect, uint32_t timeoutMs)
{
    if (!mqtt.esp8266 || !host)
        return;

    char buf[200];
    snprintf(buf, sizeof(buf), "AT+MQTTCONN=%d,\"%s\",%d,%d", mqtt.linkId, host, port, reconnect);
    mqtt.esp8266->send(buf, timeoutMs);
}

void mqtt_subscribe(const char *topic, int qos, uint32_t timeoutMs)
{
    if (!mqtt.esp8266 || !topic)
        return;

    char buf[200];
    snprintf(buf, sizeof(buf), "AT+MQTTSUB=%d,\"%s\",%d", mqtt.linkId, topic, qos);
    mqtt.esp8266->send(buf, timeoutMs);
}

void mqtt_publish(const char *topic, const char *payload, int qos, int retain, uint32_t timeoutMs)
{
    if (!mqtt.esp8266 || !topic || !payload)
        return;

    char buf[320];
    snprintf(buf, sizeof(buf), "AT+MQTTPUB=%d,\"%s\",\"%s\",%d,%d", mqtt.linkId, topic, payload, qos, retain);
    mqtt.esp8266->send(buf, timeoutMs);
}

bool mqtt_parse_recv(const char *line, size_t lineLen, MqttSubRecv *out)
{
    if (!out)
        return 0;

    const char *topic = NULL;
    const char *json = NULL;
    size_t topicLen = 0;
    size_t jsonLen = 0;
    int linkId = 0;
    int payloadLen = 0;

    const char *head = "+MQTTSUBRECV:";
    const char *p = strstr(line, head);
    if (!p)
        return false;
    p += strlen(head);

    char *endptr = NULL;
    long linkId = strtol(p, &endptr, 10);
    if (endptr == p || *endptr != ',')
        return false;
    p = endptr + 1;

    if (*p != '"')
        return false;
    const char *topicStart = ++p;
    const char *topicEnd = strchr(topicStart, '"');
    if (!topicEnd)
        return false;

    p = topicEnd + 1;
    if (*p != ',')
        return false;
    p++;

    long payloadLen = strtol(p, &endptr, 10);
    if (endptr == p || *endptr != ',')
        return false;
    p = endptr + 1;

    const char *jsonStart = p;
    size_t jsonLen = lineLen - (size_t)(jsonStart - line);
    trim_crlf(&jsonStart, &jsonLen);

    out->linkId = linkId;
    out->topic = topic;
    out->topicLen = topicLen;
    out->payloadLen = payloadLen;
    out->json = json;
    out->jsonLen = jsonLen;
    return true;
}

bool topic_equals(const char *topic, size_t topicLen, const char *expect)
{
    size_t n = strlen(expect);
    return (topicLen == n) && (memcmp(topic, expect, n) == 0);
}

int parse_int_n(const char *p, size_t n)
{
    char tmp[16];
    if (n >= sizeof(tmp))
        n = sizeof(tmp) - 1;
    memcpy(tmp, p, n);
    tmp[n] = '\0';
    return atoi(tmp);
}

static const char *SUB_TOPIC = "/sys/k21juUUmcsq/stm32/thing/service/property/set";
static const char *PUB_TOPIC = "/sys/k21juUUmcsq/stm32/thing/event/property/post";

void mqtt_init(Esp8266 *esp8266, int linkId)
{
    mqtt.linkId = linkId;
    mqtt.esp8266 = esp8266;

    memset(mqtt.mqtt_buf, 0, sizeof(mqtt.mqtt_buf));

    // AliYun:  https://help.aliyun.com/zh/iot/product-overview/limits | https://developer.aliyun.com/article/1163947
    // AT+MQTT: https://blog.csdn.net/espressif/article/details/101713780

    // Step 1: 设置 MQTT 用户信息
    // AT+MQTTUSERCFG=<LinkID>,<scheme>,<"client_id">,<"username">,<"password">,<cert_key_ID>,<CA_ID>,<"path">
    // client_id: 对应 MQTT mqtt ID, 用于标志 mqtt 身份, 最长 256 字节
    // username: 用于登录 MQTT broker 的 username, 最长 64 字节
    // password: 用于登录 MQTT broker 的 password, 最长 64 字节
    // cert_key_ID: 证书 ID, 目前支持一套 cert 证书, 参数为 0
    // CA_ID: CA ID, 目前支持一套 CA 证书, 参数为 0
    // path: 资源路径, 最长 32 字节
    const char *client_id = "k21juUUmcsq.stm32|securemode=2\\,signmethod=hmacsha256\\,timestamp=1735605362073|";
    const char *username = "stm32&k21juUUmcsq";
    const char *password = "bb60e378712ebacff26c316d7e2c695b1723626b9162a27625907d1eb4f5b037";
    mqtt_config(1, client_id, username, password, 0, 0, "", 3000);

    // Step 2: 连接 MQTT Broker
    // AT+MQTTCONN=<LinkID>,<"host">,<port>,<reconnect>
    // LinkID: 当前只支持 0
    // host: 连接 MQTT broker 域名, 最大 128 字节
    // port: 连接 MQTT broker 端口, 最大 65535
    // path: 资源路径, 最长 32 字节
    // reconnect: 是否重连 MQTT, 若设置为 1, 需要消耗较多内存资源
    const char *broker_addr = "k21juUUmcsq.iot-as-mqtt.cn-shanghai.aliyuncs.com";
    mqtt_connect(broker_addr, 1883, 1, 3000);

    // Step 3: 订阅主题
    // AT+MQTTSUB=<LinkID>,<"topic">,<qos>
    // LinkID: 当前只支持 0
    // state: MQTT 当前状态, 状态说明如下:
    // 0: 连接未初始化
    // 1: 已设置 MQTTUSERCFG
    // 2: 已设置 MQTTCONNCFG
    // 3: 连接已断开
    // 4: 已建立连接
    mqtt_subscribe(SUB_TOPIC, 0, 3000);
}

static void handle_aliyun_property_set(const char *json, size_t jsonLen)
{
    if (JSON_Validate(json, jsonLen) != JSONSuccess)
        return;

    const char *val = NULL;
    size_t valLen = 0;
    JSONTypes_t type;

    if (JSON_SearchConst(json, jsonLen, "params.led", strlen("params.led"), &val, &valLen, &type) == JSONSuccess && type == JSONNumber)
    {
        int led = parse_int_n(val, valLen);

        char payload[128];
        snprintf(payload, sizeof(payload), "{\\\"params\\\":{\\\"led\\\":%d\\}\\,\\\"version\\\":\\\"1.0.0\\\"}", led);

        if (led == 1)
            red();
        else if (led == 0)
            green();
        else
            off();

        mqtt_publish(PUB_TOPIC, payload, 0, 0, 3000);
    }

    if (JSON_SearchConst(json, jsonLen, "params.voice", strlen("params.voice"), &val, &valLen, &type) == JSONSuccess && type == JSONString)
    {
        char voice[80];
        if (valLen >= sizeof(voice))
            valLen = sizeof(voice) - 1;
        memcpy(voice, val, valLen);
        voice[valLen] = '\0';

        syn6288_send_cmd((uint8_t *)voice);

        char payload[200];
        snprintf(payload, sizeof(payload), "{\\\"params\\\":{\\\"voice\\\":\\\"%s\\\"\\}\\,\\\"version\\\":\\\"1.0.0\\\"}", voice);
        mqtt_publish(PUB_TOPIC, payload, 0, 0, 3000);
    }
}

void mqtt_start(void)
{
    memset(mqtt.mqtt_buf, 0x00, sizeof(mqtt.mqtt_buf));

    while (true)
    {
        if (mqtt.esp8266->waitRecv(0) == 0)
            continue;

        char *esp8266_buf = (char *)(mqtt.esp8266->usart2_buf);
        char *mqtt_buf = mqtt.mqtt_buf;

        uint16_t recvCount = mqtt.esp8266->usart2_count;
        size_t copyLen = recvCount;

        size_t mqttSize = sizeof(mqtt.mqtt_buf);
        if (copyLen >= mqttSize)
            copyLen = mqttSize - 1;

        memset(mqtt_buf, 0, mqttSize);
        memcpy(mqtt_buf, esp8266_buf, copyLen);
        mqtt_buf[copyLen] = '\0';

        mqtt.esp8266->clearRecv(recvCount);

        MqttSubRecv sub;
        if (mqtt_parse_recv(mqtt_buf, strlen(mqtt_buf), &sub))
        {
            if (topic_equals(sub.topic, sub.topicLen, SUB_TOPIC))
            {
                handle_aliyun_property_set(sub.json, sub.jsonLen);
            }
            else
            {
                printf("[MQTT] Unknown topic: %.*s\n", (int)sub.topicLen, sub.topic);
            }
        }
        else
        {
            printf("[MQTT] Invalid recv: %s\n", mqtt_buf);
            // +MQTTDISCONNECTED:0
        }
    }
}

// void mqtt_test(void)
// {
//     char mqtt_buf2[256];

//     // MQTT publish led status
//     // AT+MQTTPUB=0,"/sys/a1TGt6tIcAE/mqtt_stm32/thing/event/property/post","{\"params\":{\"led\":8},\"version\":\"1.0.0\"}",0,0
//     snprintf(mqtt_buf2, sizeof(mqtt_buf2), "AT+MQTTPUB=0,\"%s\",\"{\\\"params\\\":{\\\"led\\\":%d\\}\\,\\\"version\\\":\\\"1.0.0\\\"}\",%d,%d", PUB_TOPIC, 8, 0, 0);
//     printf("%s\n", mqtt_buf2);

//     // ClearArray(mqtt_buf2, sizeof(mqtt_buf2));
//     memset(mqtt_buf2, 0x00, sizeof(mqtt_buf2));

//     // MQTT publish temperature and humidity
//     // AT+MQTTPUB=0,"/sys/a1TGt6tIcAE/mqtt_stm32/thing/event/property/post","{\"params\":{\"temp\":8,\"humi\":8},\"version\":\"1.0.0\"}",0,0
//     snprintf((char *)mqtt_buf2, sizeof(mqtt_buf2), "AT+MQTTPUB=0,\"" PUB_TOPIC2 "\",\"" JSON_FORMAT "\",0,0\r\n", 8, 8);
//     printf("%s\n", mqtt_buf2);
// }
