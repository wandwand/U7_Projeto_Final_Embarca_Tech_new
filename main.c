//#include "main.h"
/*
int main() {
    stdio_init_all();
    setup_peripherals();
    setup_display();
    setup_buttons();
    setup_buzzer();
    setup();  // Configura joystick (assumindo que está em outro arquivo)
    setup_adc();
    fixed_light = false;
    update_display("  ALARME DESLIGADO", "  ADC NAO ACIONADO");
    smart_home();
}
*/


#include "hardware/structs/rosc.h"
#include <string.h>
#include <time.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "lwip/dns.h"
#include "lwip/altcp_tcp.h"
#include "lwip/altcp_tls.h"
#include "lwip/apps/mqtt.h"
#include "lwip/apps/mqtt_priv.h"

#include "main.h"

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

typedef struct MQTT_CLIENT_T_ {
    ip_addr_t remote_addr;
    mqtt_client_t *mqtt_client;
    u32_t received;
    u32_t counter;
    u32_t reconnect;
} MQTT_CLIENT_T;

err_t mqtt_test_connect(MQTT_CLIENT_T *state);

static MQTT_CLIENT_T* mqtt_client_init(void) {
    MQTT_CLIENT_T *state = calloc(1, sizeof(MQTT_CLIENT_T));
    if (!state) {
        DEBUG_printf("Failed to allocate state\n");
        return NULL;
    }
    return state;
}

void dns_found(const char *name, const ip_addr_t *ipaddr, void *callback_arg) {
    MQTT_CLIENT_T *state = (MQTT_CLIENT_T*)callback_arg;
    if (ipaddr) {
        state->remote_addr = *ipaddr;
        DEBUG_printf("DNS resolved: %s\n", ip4addr_ntoa(ipaddr));
    } else {
        DEBUG_printf("DNS resolution failed.\n");
    }
}

void run_dns_lookup(MQTT_CLIENT_T *state) {
    DEBUG_printf("Running DNS lookup for %s...\n", MQTT_SERVER_HOST);
    if (dns_gethostbyname(MQTT_SERVER_HOST, &(state->remote_addr), dns_found, state) == ERR_INPROGRESS) {
        while (state->remote_addr.addr == 0) {
            cyw43_arch_poll();
            sleep_ms(10);
        }
    }
}

static void mqtt_pub_start_cb(void *arg, const char *topic, u32_t tot_len) {
    DEBUG_printf("Incoming message on topic: %s\n", topic);
}

// main.c
static void mqtt_pub_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) {
    char buffer[BUFFER_SIZE];
    if (len < BUFFER_SIZE) {
        memcpy(buffer, data, len);
        buffer[len] = '\0';
        DEBUG_printf("Message received: %s\n", buffer);
        if (strcmp(buffer, "acender") == 0) {
            gpio_put(LED_PIN_B, 1);
            adc_enabled = true;    // Habilita o ADC
            listening = true;      // Ativa o modo de escuta
        } else if (strcmp(buffer, "apagar") == 0) {
            gpio_put(LED_PIN_B, 0);
            adc_enabled = false;   // Desliga o ADC
            listening = false;     // Desativa a escuta
            stop_beep(BUZZER_B);   // Para o buzzer
            buzzer_on = false;     // Reseta o estado do buzzer
        }
    } else {
        DEBUG_printf("Message too large, discarding.\n");
    }
}

static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    if (status == MQTT_CONNECT_ACCEPTED) {
        gpio_put(LED_PIN_G, 1);
        DEBUG_printf("MQTT connected.\n");
    } else {
        gpio_put(LED_PIN_R, 1);
        DEBUG_printf("MQTT connection failed: %d\n", status);
    }
}

void mqtt_pub_request_cb(void *arg, err_t err) {
    DEBUG_printf("Publish request status: %d\n", err);
}

void mqtt_sub_request_cb(void *arg, err_t err) {
    DEBUG_printf("Subscription request status: %d\n", err);
}

err_t mqtt_test_publish(MQTT_CLIENT_T *state) {
    char buffer[BUFFER_SIZE];
    snprintf(buffer, BUFFER_SIZE, "{\"message\":\"hello from picow %d / %d\"}", state->received, state->counter);
    return mqtt_publish(state->mqtt_client, "pico_w/test", buffer, strlen(buffer), 0, 0, mqtt_pub_request_cb, state);
}

err_t mqtt_test_connect(MQTT_CLIENT_T *state) {
    struct mqtt_connect_client_info_t ci = {0};
    ci.client_id = "PicoW";
    return mqtt_client_connect(state->mqtt_client, &(state->remote_addr), MQTT_SERVER_PORT, mqtt_connection_cb, state, &ci);
}

// main.c
void mqtt_run_test(MQTT_CLIENT_T *state) {
    state->mqtt_client = mqtt_client_new();
    if (!state->mqtt_client) {
        DEBUG_printf("Failed to create MQTT client\n");
        return;
    }

    if (mqtt_test_connect(state) == ERR_OK) {
        mqtt_set_inpub_callback(state->mqtt_client, mqtt_pub_start_cb, mqtt_pub_data_cb, NULL);
        mqtt_sub_unsub(state->mqtt_client, "pico_w/recv", 0, mqtt_sub_request_cb, NULL, 1);

        uint32_t last_publish = 0;
        while (1) {
            cyw43_arch_poll();       // Mantém a conexão WiFi
            alarme();                // Verifica o alarme e o ADC

            if (mqtt_client_is_connected(state->mqtt_client)) {
                uint32_t now = to_ms_since_boot(get_absolute_time());
                if (now - last_publish >= 5000) {
                    mqtt_test_publish(state);  // Publica a cada 5s
                    last_publish = now;
                }
            } else {
                DEBUG_printf("Reconnecting...\n");
                sleep_ms(1000);
                mqtt_test_connect(state);
            }
            sleep_ms(10);  // Evita uso excessivo da CPU
        }
    }
}



int main() {
    stdio_init_all();
    setup_buzzer();
    setup_adc();
    
    

    gpio_init(LED_PIN_R);
    gpio_init(LED_PIN_B);
    gpio_init(LED_PIN_G);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);
    gpio_set_dir(LED_PIN_B, GPIO_OUT);
    gpio_set_dir(LED_PIN_G, GPIO_OUT);

    if (cyw43_arch_init()) {
        DEBUG_printf("Failed to initialize WiFi\n");
        return 1;
    }
    cyw43_arch_enable_sta_mode();

    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        DEBUG_printf("Failed to connect to WiFi\n");
        return 1;
    }

    MQTT_CLIENT_T *state = mqtt_client_init();
    run_dns_lookup(state);
    while(1){
        mqtt_run_test(state);
        alarme();
    }
    
    

    cyw43_arch_deinit();
    return 0;
}
