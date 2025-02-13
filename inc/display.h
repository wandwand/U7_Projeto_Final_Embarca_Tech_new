#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "inc/ssd1306.h"
#include "pico/binary_info.h"


//variaveis display
#define SAMPLES 200
extern uint16_t adc_buffer[SAMPLES];
extern uint8_t ssd[ssd1306_buffer_length];
#define I2C_SDA 14
#define I2C_SCL 15
#define ADC_ADJUST(x) (x * 3.3f / (1 << 12u) - 1.65f)

// Área de renderização do display
extern struct render_area frame_area;


//Prototipo
void update_display(const char *line1, const char *line2);
void setup_display();
void setup_peripherals();

#endif