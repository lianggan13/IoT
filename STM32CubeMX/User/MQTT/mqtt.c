#include "mqtt.h"

#include <stdio.h>

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

void mqtt_client_config(MqttClient *client, int scheme,
                        const char *clientId, const char *username, const char *password,
                        int certKeyId, int caId, const char *path,
                        uint32_t timeoutMs)
{
    if (!client)
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
             client->linkId, scheme, clientId, username, password, certKeyId, caId, path);
    client->send(buf, timeoutMs);
}

void mqtt_client_connect(MqttClient *client, const char *host, int port, int reconnect, uint32_t timeoutMs)
{
    if (!client || !host)
        return;

    char buf[200];
    snprintf(buf, sizeof(buf), "AT+MQTTCONN=%d,\"%s\",%d,%d", client->linkId, host, port, reconnect);
    client->send(buf, timeoutMs);
}

void mqtt_client_subscribe(MqttClient *client, const char *topic, int qos, uint32_t timeoutMs)
{
    if (!client || !topic)
        return;

    char buf[200];
    snprintf(buf, sizeof(buf), "AT+MQTTSUB=%d,\"%s\",%d", client->linkId, topic, qos);
    client->send(buf, timeoutMs);
}

void mqtt_client_publish(MqttClient *client, const char *topic, const char *payload, int qos, int retain, uint32_t timeoutMs)
{
    if (!client || !topic || !payload)
        return;

    char buf[320];
    snprintf(buf, sizeof(buf), "AT+MQTTPUB=%d,\"%s\",\"%s\",%d,%d", client->linkId, topic, payload, qos, retain);
    client->send(buf, timeoutMs);
}

uint8_t mqtt_parse_subrecv(
    const char *line, size_t lineLen,
    int *outLinkId,
    const char **outTopic, size_t *outTopicLen,
    int *outPayloadLen,
    const char **outJson, size_t *outJsonLen)
{
    const char *p = strstr(line, "+MQTTSUBRECV:");
    if (!p)
        return 0;
    p += strlen("+MQTTSUBRECV:");

    char *endptr = NULL;
    long linkId = strtol(p, &endptr, 10);
    if (endptr == p || *endptr != ',')
        return 0;
    p = endptr + 1;

    if (*p != '"')
        return 0;
    const char *topicStart = ++p;
    const char *topicEnd = strchr(topicStart, '"');
    if (!topicEnd)
        return 0;

    p = topicEnd + 1;
    if (*p != ',')
        return 0;
    p++;

    long payloadLen = strtol(p, &endptr, 10);
    if (endptr == p || *endptr != ',')
        return 0;
    p = endptr + 1;

    const char *jsonStart = p;
    size_t jsonLen = lineLen - (size_t)(jsonStart - line);
    trim_crlf(&jsonStart, &jsonLen);

    *outLinkId = (int)linkId;
    *outTopic = topicStart;
    *outTopicLen = (size_t)(topicEnd - topicStart);
    *outPayloadLen = (int)payloadLen;
    *outJson = jsonStart;
    *outJsonLen = jsonLen;
    return 1;
}

uint8_t mqtt_client_parse_subrecv(const char *line, size_t lineLen, MqttSubRecv *out)
{
    if (!out)
        return 0;

    const char *topic = NULL;
    const char *json = NULL;
    size_t topicLen = 0;
    size_t jsonLen = 0;
    int linkId = 0;
    int payloadLen = 0;

    if (!mqtt_parse_subrecv(line, lineLen, &linkId, &topic, &topicLen, &payloadLen, &json, &jsonLen))
        return 0;

    out->linkId = linkId;
    out->topic = topic;
    out->topicLen = topicLen;
    out->payloadLen = payloadLen;
    out->json = json;
    out->jsonLen = jsonLen;
    return 1;
}

uint8_t topic_equals(const char *topic, size_t topicLen, const char *expect)
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

void mqtt_client_init(MqttClient *client, int linkId, Send send)
{
    client->linkId = linkId;

    client->send = send;
    client->parseSubRecv = mqtt_client_parse_subrecv;

    client->config = mqtt_client_config;
    client->connect = mqtt_client_connect;
    client->subscribe = mqtt_client_subscribe;
    client->publish = mqtt_client_publish;
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
