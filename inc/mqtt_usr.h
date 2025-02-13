#ifndef MQTT_USR_INC_H
#define MQTT_USR_INC_H

#include "pico/binary_info.h"
#include "pico/stdlib.h"
#include "hardware/structs/rosc.h"
#include <string.h>
#include <time.h>

#include "pico/cyw43_arch.h"

#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "lwip/dns.h"
#include "lwip/altcp_tcp.h"
#include "lwip/altcp_tls.h"
#include "lwip/apps/mqtt.h"
#include "lwip/apps/mqtt_priv.h"


//Variaveis MQTT
#define LED_PIN_G 11
#define LED_PIN_B 12
#define LED_PIN_R 13
#define DEBUG_printf printf
#define MQTT_SERVER_HOST "broker.emqx.io"  
#define MQTT_SERVER_PORT 1883
#define MQTT_TLS 0
#define WIFI_SSID "Casa27.1 2g" 
#define WIFI_PASSWORD "7707407944"

/*
WIFI_SSID = SSID da sua rede Wifi;
WIFI_PASSWORD = Senha do Wifi;
MQTT_SERVER_HOST = endereço MQTT (ex:broker.emqx.io);
MQTT_SERVER_PORT = Número da porta (ex:1883).
*/

#define BUFFER_SIZE 256

typedef struct {
    ip_addr_t remote_addr;
    mqtt_client_t *mqtt_client;
    u32_t received;
    u32_t counter;
    u32_t reconnect;
} MQTT_CLIENT_T;

// Protótipos das funções movidas para mqtt.c

MQTT_CLIENT_T* mqtt_client_init(void);
void dns_found(const char *name, const ip_addr_t *ipaddr, void *callback_arg);
void run_dns_lookup(MQTT_CLIENT_T *state);
void mqtt_pub_start_cb(void *arg, const char *topic, u32_t tot_len);
void mqtt_pub_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags);
void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status);
void mqtt_pub_request_cb(void *arg, err_t err);
void mqtt_sub_request_cb(void *arg, err_t err);
err_t mqtt_test_publish(MQTT_CLIENT_T *state);
err_t mqtt_test_connect(MQTT_CLIENT_T *state);
void mqtt_run_test(MQTT_CLIENT_T *state);


#endif  //MQTT_USR_INC_H