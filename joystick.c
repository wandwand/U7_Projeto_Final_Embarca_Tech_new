#include "main.h"

// Definição dos pinos usados para o joystick e LEDs
const int VRX = 26;          // Pino de leitura do eixo X do joystick (ADC0)
const int VRY = 27;          // Pino de leitura do eixo Y do joystick (ADC1)
const int ADC_CHANNEL_0 = 0; // Canal ADC para o eixo X
const int ADC_CHANNEL_1 = 1; // Canal ADC para o eixo Y
const int LED_B = 13;        // LED azul (PWM)
const int LED_R = 11;        // LED vermelho (PWM)
const float DIVIDER_PWM = 16.0;
const uint16_t PERIOD = 4096;
uint16_t led_b_level = 100, led_r_level = 100;
uint slice_led_b, slice_led_r;

static uint16_t vrx_value, vry_value;

// Protótipos
void setup_joystick();
void setup_pwm_led(uint led, uint *slice, uint16_t level);
void setup();
void joystick_read_axis(uint16_t *vrx_value, uint16_t *vry_value);

void joystick(bool fixed_light_t) {
    if (!fixed_light_t) {
        joystick_read_axis(&vrx_value, &vry_value);
        pwm_set_gpio_level(LED_B, vrx_value);
        pwm_set_gpio_level(LED_R, vry_value);
    }
    sleep_ms(10);
}

void setup_joystick() {
    adc_init();         // Inicializa o ADC uma única vez
    adc_gpio_init(VRX);
    adc_gpio_init(VRY);

    gpio_init(SW);
    gpio_set_dir(SW, GPIO_IN);
    gpio_pull_up(SW);
    gpio_set_irq_enabled_with_callback(SW, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
}

void setup_pwm_led(uint led, uint *slice, uint16_t level) {
    gpio_set_function(led, GPIO_FUNC_PWM);
    *slice = pwm_gpio_to_slice_num(led);
    pwm_set_clkdiv(*slice, DIVIDER_PWM);
    pwm_set_wrap(*slice, PERIOD);
    pwm_set_gpio_level(led, level);
    pwm_set_enabled(*slice, true);
}

void setup() {
    stdio_init_all();
    setup_joystick();
    setup_pwm_led(LED_B, &slice_led_b, led_b_level);
    setup_pwm_led(LED_R, &slice_led_r, led_r_level);
}

void joystick_read_axis(uint16_t *vrx_value, uint16_t *vry_value) {
    adc_select_input(ADC_CHANNEL_0);
    sleep_us(2);
    *vrx_value = adc_read();

    adc_select_input(ADC_CHANNEL_1);
    sleep_us(2);
    *vry_value = adc_read();
}