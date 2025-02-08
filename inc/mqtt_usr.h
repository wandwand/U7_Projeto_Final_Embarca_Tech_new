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

#include "main.h"
//Variaveis MQTT
#define LED_PIN_G 11
#define LED_PIN_B 12
#define LED_PIN_R 13
#define DEBUG_printf printf
#define MQTT_SERVER_HOST "broker.emqx.io"
#define MQTT_SERVER_PORT 1883
#define MQTT_TLS 0
#define WIFI_SSID "Uruguai" 
#define WIFI_PASSWORD "7707407944" 
#define BUFFER_SIZE 256

typedef struct {
    ip_addr_t remote_addr;
    mqtt_client_t *mqtt_client;
    u32_t received;
    u32_t counter;
    u32_t reconnect;
} MQTT_CLIENT_T;

// Protótipos das funções movidas para mqtt.c

/**
 * @brief Inicializa um cliente MQTT.
 * @return Ponteiro para a estrutura MQTT_CLIENT_T inicializada, ou NULL em caso de erro.
 */
MQTT_CLIENT_T* mqtt_client_init(void);

/**
 * @brief Callback chamada quando a resolução de DNS é concluída.
 * @param name Nome do host resolvido.
 * @param ipaddr Endereço IP resolvido.
 * @param callback_arg Argumento passado para o callback (ponteiro para a estrutura MQTT_CLIENT_T).
 */
void dns_found(const char *name, const ip_addr_t *ipaddr, void *callback_arg);

/**
 * @brief Realiza a busca de DNS para obter o IP do servidor MQTT.
 * @param state Ponteiro para a estrutura MQTT_CLIENT_T.
 */
void run_dns_lookup(MQTT_CLIENT_T *state);

/**
 * @brief Callback chamada quando uma nova publicação MQTT é recebida.
 * @param arg Argumento do callback.
 * @param topic Tópico da mensagem recebida.
 * @param tot_len Tamanho total da mensagem.
 */
void mqtt_pub_start_cb(void *arg, const char *topic, u32_t tot_len);

/**
 * @brief Callback chamada quando os dados de uma publicação MQTT são recebidos.
 * @param arg Argumento do callback.
 * @param data Dados recebidos.
 * @param len Tamanho dos dados.
 * @param flags Flags da mensagem MQTT.
 */
void mqtt_pub_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags);

/**
 * @brief Callback chamada quando a conexão MQTT é estabelecida ou falha.
 * @param client Ponteiro para a estrutura mqtt_client_t.
 * @param arg Argumento do callback.
 * @param status Status da conexão MQTT.
 */
void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status);

/**
 * @brief Callback chamada quando uma publicação MQTT é realizada.
 * @param arg Argumento do callback.
 * @param err Código de erro da publicação.
 */
void mqtt_pub_request_cb(void *arg, err_t err);

/**
 * @brief Callback chamada quando uma solicitação de inscrição MQTT é concluída.
 * @param arg Argumento do callback.
 * @param err Código de erro da inscrição.
 */
void mqtt_sub_request_cb(void *arg, err_t err);

/**
 * @brief Publica uma mensagem de teste no tópico MQTT "pico_w/test".
 * @param state Ponteiro para a estrutura MQTT_CLIENT_T.
 * @return Código de erro da publicação MQTT.
 */
err_t mqtt_test_publish(MQTT_CLIENT_T *state);

/**
 * @brief Conecta ao broker MQTT.
 * @param state Ponteiro para a estrutura MQTT_CLIENT_T.
 * @return Código de erro da conexão MQTT.
 */
err_t mqtt_test_connect(MQTT_CLIENT_T *state);

/**
 * @brief Inicializa e executa o cliente MQTT, publicando mensagens periodicamente.
 * @param state Ponteiro para a estrutura MQTT_CLIENT_T.
 */
void mqtt_run_test(MQTT_CLIENT_T *state);


#endif