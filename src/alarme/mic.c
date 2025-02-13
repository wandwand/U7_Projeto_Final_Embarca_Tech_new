#include "inc\mic.h"

uint16_t adc_buffer_mic[SAMPLES_MIC];  // Definição da variável

// Configuração do ADC para o microfone
void setup_adc()
{
    adc_init(); 
    adc_gpio_init(MIC_PIN); 
    adc_select_input(MIC_CHANNEL);
    adc_set_clkdiv(ADC_CLOCK_DIV); 
}

// Coleta amostras do microfone
void sample_mic()
{
    for (uint i = 0; i < SAMPLES_MIC; ++i)
    {
        adc_select_input(MIC_CHANNEL); // Define canal explicitamente
        sleep_us(2);
        adc_buffer_mic[i] = adc_read();
    }
}

// Calcula a potência média do sinal do microfone
float mic_power()
{
    float avg = 0.f;
    for (uint i = 0; i < SAMPLES_MIC; ++i)
        avg += adc_buffer_mic[i] * adc_buffer_mic[i];
    return sqrt(avg / SAMPLES_MIC);
}

// Obtém a intensidade do som com base na potência
uint8_t get_intensity(float v)
{
    if (v < 0.2f)
        return 0;
    if (v < 0.4f)
        return 1;
    if (v < 0.6f)
        return 2;
    if (v < 0.8f)
        return 3;
    return 4;
}