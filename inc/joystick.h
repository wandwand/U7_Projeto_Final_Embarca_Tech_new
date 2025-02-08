#ifndef joystick_h
#define joystick_h

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "pico/binary_info.h"

//Variaveis joystick
#define SW 22             // Pino de leitura do botão do joystick
static bool fixed_light;    // Variável para verificar se a luz está fixa ou não
extern bool leds_enabled;

// Protótipos das funções movidas para joystick.c
void setup();
void joystick(bool fixed_light_t);
void gpio_callback(uint gpio, uint32_t events);

#endif