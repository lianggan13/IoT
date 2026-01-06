#ifndef MQTT_H
#define MQTT_H

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>

typedef struct MqttClient MqttClient;
typedef struct MqttSubRecv MqttSubRecv;

typedef void (*Send)(char *cmd, uint32_t timeoutMs);
typedef uint8_t (*ParseSubRecv)(const char *line, size_t lineLen, MqttSubRecv *out);

typedef void (*Config)(MqttClient *client, int scheme, const char *clientId, const char *username, const char *password,
                       int certKeyId, int caId, const char *path,
                       uint32_t timeoutMs);

typedef void (*Connect)(MqttClient *client, const char *host, int port, int reconnect, uint32_t timeoutMs);
typedef void (*Subscribe)(MqttClient *client, const char *topic, int qos, uint32_t timeoutMs);
typedef void (*Publish)(MqttClient *client, const char *topic, const char *payload, int qos, int retain, uint32_t timeoutMs);

struct MqttClient
{
    int linkId;

    Send send;
    ParseSubRecv parseSubRecv;

    Config config;
    Connect connect;
    Subscribe subscribe;
    Publish publish;
};

struct MqttSubRecv
{
    int linkId;
    const char *topic;
    size_t topicLen;
    int payloadLen;
    const char *json;
    size_t jsonLen;
};

void mqtt_client_init(MqttClient *client, int linkId, Send send);
uint8_t topic_equals(const char *topic, size_t topicLen, const char *expect);
int parse_int_n(const char *p, size_t n);

#endif
