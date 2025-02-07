#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "pico/binary_info.h"

//Variaveis joystick
#define SW 22             // Pino de leitura do botão do joystick
static bool fixed_light;    // Variável para verificar se a luz está fixa ou não
extern bool leds_enabled;

// Variaveis alarme
#define SAMPLES 200
#define BUZZER_B 10   // Buzzer B 
extern bool leds_enabled;
extern volatile bool adc_enabled;
extern volatile bool buzzer_on;
extern volatile bool listening;
extern uint16_t adc_buffer[SAMPLES];
extern uint8_t ssd[ssd1306_buffer_length];
extern uint64_t last_button_5_time;
extern uint64_t last_button_6_time;
extern uint64_t last_sw_time;

// Protótipos das funções movidas para alarme.c
void setup_peripherals();
void setup_display();
void setup_adc();
void setup_buttons();
void setup_buzzer();
void sample_mic();
float mic_power();
uint8_t get_intensity(float v);
void beep(uint pin);
void stop_beep(uint pin);
void update_display(const char *line1, const char *line2);
void gpio_callback(uint gpio, uint32_t events);
void alarme();
void smart_home();


// Protótipos das funções movidas para joystick.c
void setup();
void joystick(bool fixed_light_t);
void gpio_callback(uint gpio, uint32_t events);