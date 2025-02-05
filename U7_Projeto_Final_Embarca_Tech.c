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
    update_display("  ALARME DESLIGADO", "  ADC NAO ACIONADO");
    smart_home();
}