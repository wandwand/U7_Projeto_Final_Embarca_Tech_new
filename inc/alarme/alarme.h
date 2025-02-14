#ifndef ALARME_H
#define ALARME_H

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"

//Alarme
#define BUZZER_B 10 // Buzzer B
#define BUZZER_FREQUENCY 100
#define BUTTON_5_PIN 5


extern volatile bool buzzer_on;
extern bool leds_enabled;


extern uint64_t last_button_5_time;
extern uint64_t last_button_6_time;
extern uint64_t last_sw_time;



// Protótipos das funções movidas para alarme.c

void alarme(void);
void smart_home(void);

void setup_adc(void);
void setup_buttons(void);
void setup_buzzer(void);
void gpio_callback(uint gpio, uint32_t events);

void beep(uint pin);
void stop_beep(uint pin);


#endif // ALARME_H