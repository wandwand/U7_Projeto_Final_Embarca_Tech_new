/*
#include "main.h"

int main() {
    stdio_init_all();
    setup_peripherals();
    setup_display();
    setup_buttons();
    setup_buzzer();
    setup();  // Configura joystick (assumindo que está em outro arquivo)
    setup_adc();
    fixed_light = false;
    update_display("  ALARME OFF", "  ADC DISABLED");
    smart_home();
}

*/

#include "main.h"


int main() {
    stdio_init_all();
    setup_buzzer();
    setup_adc();
    setup_led_rgb();
    setup_peripherals();
    setup_display();
    setup_buttons();
   
    fixed_light = false;
    update_display("  ALARME OFF", "  ADC DISABLED");
    

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
        
    }
    
    

    cyw43_arch_deinit();
    return 0;
}
