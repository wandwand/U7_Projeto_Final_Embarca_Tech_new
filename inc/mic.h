#ifndef MIC_H
#define MIC_H

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
#include "pico/binary_info.h"

//Microfone
#define MIC_CHANNEL 2
#define MIC_PIN (26 + MIC_CHANNEL)
#define ADC_CLOCK_DIV 256.f // Clock seguro (~488 kHz)
#define SAMPLES_MIC 200
extern uint16_t adc_buffer_mic[SAMPLES_MIC];  // Definição real

extern volatile bool mic_enabled;

extern volatile bool listening;


#endif