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
#include "inc\config.h"


// Protótipos das funções movidas para joystick.c


void joystick(bool fixed_light_t);
void setup();
void gpio_callback(uint gpio, uint32_t events); 

#endif //JOYSTICK_H