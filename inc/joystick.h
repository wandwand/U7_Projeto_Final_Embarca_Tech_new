#ifndef JOYSTICK_H
#define JOYSTICK_H

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
/**
 * @brief Controla os LEDs com base na posição do joystick.
 * @param fixed_light_t Se verdadeiro, mantém os LEDs fixos; caso contrário, ajusta com base nos eixos do joystick.
 */
void joystick(bool fixed_light_t);

/**
 * @brief Configura os periféricos necessários para o funcionamento do joystick e LEDs.
 */
void setup();

/**
 * @brief Callback para interrupções dos botões, alterando estados do alarme e LEDs.
 * @param gpio Número do pino que acionou a interrupção.
 * @param events Tipo de evento ocorrido (borda de subida/descida).
 */
void gpio_callback(uint gpio, uint32_t events); 

#endif //JOYSTICK_H