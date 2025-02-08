#ifndef ALARME_H
#define ALARME_H

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


/**
 * @brief Monitora o microfone e ativa o alarme caso um som acima do limite seja detectado.
 */
void alarme(void);

/**
 * @brief Executa continuamente a função do joystick e do alarme, simulando um ambiente de casa inteligente.
 */
void smart_home(void);

/**
 * @brief Inicializa a comunicação I2C para o display OLED e configura os pinos correspondentes.
 */
void setup_peripherals(void);

/**
 * @brief Inicializa e configura o display OLED.
 */
void setup_display(void);

/**
 * @brief Configura o ADC para leitura do microfone, ajustando o clock para evitar ruídos.
 */
void setup_adc(void);

/**
 * @brief Configura os botões de controle e associa interrupções para acionamento.
 */
void setup_buttons(void);

/**
 * @brief Configura o buzzer PWM para emissão de alertas sonoros.
 */
void setup_buzzer(void);

/**
 * @brief Callback para interrupções dos botões, alterando estados do alarme e LEDs.
 * @param gpio Número do pino que acionou a interrupção.
 * @param events Tipo de evento ocorrido (borda de subida/descida).
 */
void gpio_callback(uint gpio, uint32_t events);

/**
 * @brief Coleta amostras do microfone e armazena no buffer para análise posterior.
 */
void sample_mic(void);

/**
 * @brief Calcula a potência do sinal do microfone a partir das amostras coletadas.
 * @return Potência média do sinal de áudio.
 */
float mic_power(void);

/**
 * @brief Converte a potência do som detectado em um nível de intensidade de 0 a 4.
 * @param v Valor da potência calculada.
 * @return Intensidade categorizada entre 0 e 4.
 */
uint8_t get_intensity(float v);

/**
 * @brief Ativa o buzzer configurado no pino especificado.
 * @param pin Pino do buzzer que será ativado.
 */
void beep(uint pin);

/**
 * @brief Desativa o buzzer configurado no pino especificado.
 * @param pin Pino do buzzer que será desativado.
 */
void stop_beep(uint pin);

/**
 * @brief Atualiza o display OLED com novas mensagens na tela.
 * @param line1 Texto para a primeira linha do display.
 * @param line2 Texto para a segunda linha do display.
 */
void update_display(const char *line1, const char *line2);

#endif // ALARME_H