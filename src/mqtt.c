#include "inc\mqtt_usr.h"
#include "inc\alarme\alarme.h"
#include "inc\alarme\mic.h"


/**
 * @brief Inicializa um cliente MQTT.
 * @return Ponteiro para a estrutura MQTT_CLIENT_T inicializada, ou NULL em caso de erro.
 */
MQTT_CLIENT_T *mqtt_client_init(void) {
    MQTT_CLIENT_T *state = calloc(1, sizeof(MQTT_CLIENT_T));
    if (!state) {
        DEBUG_printf("Failed to allocate state\n");
        return NULL;
    }
    return state;
}

/**
 * @brief Callback chamada quando a resolução de DNS é concluída.
 * @param name Nome do host resolvido.
 * @param ipaddr Endereço IP resolvido.
 * @param callback_arg Argumento passado para o callback (ponteiro para a estrutura MQTT_CLIENT_T).
 */
void dns_found(const char *name, const ip_addr_t *ipaddr, void *callback_arg) {
    MQTT_CLIENT_T *state = (MQTT_CLIENT_T *)callback_arg;
    if (ipaddr) {
        state->remote_addr = *ipaddr;
        DEBUG_printf("DNS resolved: %s\n", ip4addr_ntoa(ipaddr));
    } else {
        DEBUG_printf("DNS resolution failed.\n");
    }
}

/**
 * @brief Realiza a busca de DNS para obter o IP do servidor MQTT.
 * @param state Ponteiro para a estrutura MQTT_CLIENT_T.
 */
void run_dns_lookup(MQTT_CLIENT_T *state) {
    DEBUG_printf("Running DNS lookup for %s...\n", MQTT_SERVER_HOST);
    if (dns_gethostbyname(MQTT_SERVER_HOST, &(state->remote_addr), dns_found, state) == ERR_INPROGRESS) {
        while (state->remote_addr.addr == 0) {
            cyw43_arch_poll();
            sleep_ms(10);
        }
    }
}

/**
 * @brief Callback chamada quando uma nova publicação MQTT é recebida.
 * @param arg Argumento do callback.
 * @param topic Tópico da mensagem recebida.
 * @param tot_len Tamanho total da mensagem.
 */
void mqtt_pub_start_cb(void *arg, const char *topic, u32_t tot_len) {
    DEBUG_printf("Incoming message on topic: %s\n", topic);
}

/**
 * @brief Callback chamada quando os dados de uma publicação MQTT são recebidos.
 * @param arg Argumento do callback.
 * @param data Dados recebidos.
 * @param len Tamanho dos dados.
 * @param flags Flags da mensagem MQTT.
 */
void mqtt_pub_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) {
    char buffer[BUFFER_SIZE];
    if (len < BUFFER_SIZE) {
        memcpy(buffer, data, len);
        buffer[len] = '\0';
        DEBUG_printf("Message received: %s\n", buffer);

        // Controle do alarme via MQTT
        if (strcmp(buffer, "liga") == 0) {
            gpio_put(LED_PIN_B, 1);
            listening = true;
            mic_enabled = true;
            update_display("ALARME OFF", "ADC ENABLED");
        } else if (strcmp(buffer, "desliga") == 0) {
            gpio_put(LED_PIN_B, 0);
            stop_beep(BUZZER_B);
            mic_enabled = false;
            update_display("ALARME OFF", "ADC DISABLED");
        }
    } else {
        DEBUG_printf("Message too large, discarding.\n");
    }
}

/**
 * @brief Callback chamada quando a conexão MQTT é estabelecida ou falha.
 * @param client Ponteiro para a estrutura mqtt_client_t.
 * @param arg Argumento do callback.
 * @param status Status da conexão MQTT.
 */
void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    if (status == MQTT_CONNECT_ACCEPTED) {
        gpio_put(LED_PIN_G, 1);
        DEBUG_printf("MQTT connected.\n");
    } else {
        gpio_put(LED_PIN_R, 1);
        DEBUG_printf("MQTT connection failed: %d\n", status);
    }
}

/**
 * @brief Callback chamada quando uma publicação MQTT é realizada.
 * @param arg Argumento do callback.
 * @param err Código de erro da publicação.
 */
void mqtt_pub_request_cb(void *arg, err_t err) {
    DEBUG_printf("Publish request status: %d\n", err);
}

/**
 * @brief Callback chamada quando uma solicitação de inscrição MQTT é concluída.
 * @param arg Argumento do callback.
 * @param err Código de erro da inscrição.
 */
void mqtt_sub_request_cb(void *arg, err_t err) {
    DEBUG_printf("Subscription request status: %d\n", err);
}
 
/**
 * @brief Publica uma mensagem de teste no tópico MQTT "pico_w/test".
 * @param state Ponteiro para a estrutura MQTT_CLIENT_T.
 * @return Código de erro da publicação MQTT.
 */
err_t mqtt_test_publish(MQTT_CLIENT_T *state) {
    char buffer[BUFFER_SIZE];
   
    snprintf(buffer, BUFFER_SIZE, "{\"message\":\"alarme disparando %d / %d\"}", state->received, state->counter);
    return mqtt_publish(state->mqtt_client, "pico_w/test", buffer, strlen(buffer), 0, 0, mqtt_pub_request_cb, state);
}

/**
 * @brief Conecta ao broker MQTT.
 * @param state Ponteiro para a estrutura MQTT_CLIENT_T.
 * @return Código de erro da conexão MQTT.
 */
err_t mqtt_test_connect(MQTT_CLIENT_T *state)
{
    struct mqtt_connect_client_info_t ci = {0};
    ci.client_id = "PicoW";
    return mqtt_client_connect(state->mqtt_client, &(state->remote_addr), MQTT_SERVER_PORT, mqtt_connection_cb, state, &ci);
}


/**
 * @brief Inicializa e executa o cliente MQTT, publicando mensagens periodicamente.
 * @param state Ponteiro para a estrutura MQTT_CLIENT_T.
 */
void mqtt_run_test(MQTT_CLIENT_T *state)
{
    state->mqtt_client = mqtt_client_new();
    if (!state->mqtt_client)
    {
        DEBUG_printf("Failed to create MQTT client\n");
        return;
    }

    if (mqtt_test_connect(state) == ERR_OK)
    {
        mqtt_set_inpub_callback(state->mqtt_client, mqtt_pub_start_cb, mqtt_pub_data_cb, NULL);
        mqtt_sub_unsub(state->mqtt_client, "pico_w/recv", 0, mqtt_sub_request_cb, NULL, 1);

        while (1)
        {
            cyw43_arch_poll();
            if (mqtt_client_is_connected(state->mqtt_client))
            {

                alarme();

                if (buzzer_on)
                {
                    mqtt_test_publish(state);
                }
            }
            else
            {
                DEBUG_printf("Reconnecting...\n");
                sleep_ms(1000);
                mqtt_test_connect(state);
            }
        }
    }
}