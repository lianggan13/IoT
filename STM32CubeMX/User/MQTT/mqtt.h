#ifndef MQTT_H
#define MQTT_H

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "core_json.h"
#include "esp8266.h"

typedef struct Mqtt Mqtt;
typedef struct MqttSubRecv MqttSubRecv;

struct Mqtt
{
    int linkId;
    char mqtt_buf[256];
    Esp8266 *esp8266;
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

extern Mqtt mqtt;

void mqtt_init(Esp8266 *esp8266, int linkId);

void mqtt_start(void);

#endif
