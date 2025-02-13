#include "inc\alarme\mic.h"

uint16_t adc_buffer_mic[SAMPLES_MIC];  // Definição da variável

/**
 * @brief Configura o ADC para leitura do microfone.
 * 
 * Inicializa o ADC, configura o pino do microfone e define o canal de entrada.
 * Também ajusta o divisor de clock para garantir uma taxa de amostragem segura.
 */
void setup_adc()
{
    adc_init(); 
    adc_gpio_init(MIC_PIN); 
    adc_select_input(MIC_CHANNEL);
    adc_set_clkdiv(ADC_CLOCK_DIV); 
}

/**
 * @brief Coleta amostras do microfone e armazena no buffer ADC.
 * 
 * Realiza múltiplas leituras do ADC no canal do microfone e armazena os valores 
 * no buffer `adc_buffer_mic`. Cada amostra tem um pequeno atraso para estabilidade.
 */
void sample_mic()
{
    for (uint i = 0; i < SAMPLES_MIC; ++i)
    {
        adc_select_input(MIC_CHANNEL); // Define canal explicitamente
        sleep_us(2);
        adc_buffer_mic[i] = adc_read();
    }
}

/**
 * @brief Calcula a potência média do sinal captado pelo microfone.
 * 
 * A potência é calculada somando os quadrados das amostras armazenadas no buffer 
 * e dividindo pelo número total de amostras. O resultado representa a intensidade 
 * média do sinal captado.
 * 
 * @return Potência média do sinal do microfone.
 */
float mic_power()
{
    float avg = 0.f;
    for (uint i = 0; i < SAMPLES_MIC; ++i)
        avg += adc_buffer_mic[i] * adc_buffer_mic[i];
    return sqrt(avg / SAMPLES_MIC);
}

/**
 * @brief Determina a intensidade do som com base na potência calculada.
 * 
 * Classifica a intensidade do som em níveis de 0 a 4, dependendo do valor da 
 * potência média calculada.
 * 
 * @param v Potência do sinal calculada pela função mic_power().
 * @return Nível de intensidade do som (0 a 4).
 */
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
