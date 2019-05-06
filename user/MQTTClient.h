#ifndef MQTTCLIENT_H
#define MQTTCLIENT_H

#include "lwip/apps/mqtt.h"

void example_do_connect(mqtt_client_t *client);
void example_publish(mqtt_client_t *client, void *arg);

#endif

