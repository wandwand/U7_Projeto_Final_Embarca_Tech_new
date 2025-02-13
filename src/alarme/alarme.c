#include "main.h"
#include "inc\display.h"
#include "inc\alarme\mic.h"

#define DEBOUNCE_DELAY_US 50000

// Variáveis globais 
bool leds_enabled = true;
volatile bool mic_enabled = false;
volatile bool listening = false;

volatile bool buzzer_on = false;


uint64_t last_button_5_time = 0;
uint64_t last_button_6_time = 0;
uint64_t last_sw_time = 0;




/**
 * @brief Função principal do alarme.
 * 
 * Verifica se o microfone está habilitado e, caso esteja, realiza a leitura do som ambiente.
 * Se a intensidade do som for crítica (>= 2), dispara o alarme (buzzer) e atualiza o display.
 * Caso contrário, apenas imprime a intensidade do som no console.
 */
void alarme()
{
    if (mic_enabled)
    {
        sample_mic();
        float avg = mic_power();
        avg = 2.f * fabs(ADC_ADJUST(avg));
        uint8_t intensity = get_intensity(avg);

        if (intensity >= 2 && !buzzer_on)
        {
            beep(BUZZER_B);
            buzzer_on = true;
            listening = true;
            update_display("  ALARME ON", " ADC ENABLED");
            printf("Intensidade crítica: %d (Alarme disparado)\n", intensity); // Aviso único
        }
        else if (intensity <= 2)
        { // Só imprime intensidades normais
            printf("Intensidade: %d\n", intensity);
        }

        if (buzzer_on)
        {
            printf("Alarme em disparo \n");
        }
    }
    // sleep_ms(10);
}

/**
 * @brief Função principal da smart home.
 * 
 * Executa um loop infinito que monitora o joystick e o alarme.
 */
void smart_home()
{
    while (true)
    {
        joystick(fixed_light);
        alarme();
    }
}

/**
 * @brief Configura os botões do sistema.
 * 
 * Inicializa os pinos dos botões (BUTTON_5_PIN e BUTTON_6_PIN) como entradas com pull-up.
 * Habilita interrupções para detectar bordas de descida (botão pressionado) e associa uma função de callback.
 */
void setup_buttons()
{
    gpio_init(BUTTON_5_PIN);
    gpio_set_dir(BUTTON_5_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_5_PIN);

    gpio_set_irq_enabled_with_callback(BUTTON_5_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled(BUTTON_6_PIN, GPIO_IRQ_EDGE_FALL, true);
}

/**
 * @brief Configura os LEDs RGB.
 * 
 * Inicializa os pinos dos LEDs (LED_PIN_R, LED_PIN_G, LED_PIN_B) como saídas.
 */
void setup_led_rgb()
{
    gpio_init(LED_PIN_R);
    gpio_init(LED_PIN_B);
    gpio_init(LED_PIN_G);

    gpio_set_dir(LED_PIN_R, GPIO_OUT);
    gpio_set_dir(LED_PIN_B, GPIO_OUT);
    gpio_set_dir(LED_PIN_G, GPIO_OUT);
}

/**
 * @brief Configura o buzzer.
 * 
 * Inicializa o pino do buzzer (BUZZER_B) como saída PWM.
 * Configura a frequência do buzzer e o nível inicial (desligado).
 */
void setup_buzzer()
{
    // Buzzer B
    gpio_init(BUZZER_B);
    gpio_set_function(BUZZER_B, GPIO_FUNC_PWM);
    uint slice_2_num = pwm_gpio_to_slice_num(BUZZER_B);
    pwm_config config_2 = pwm_get_default_config();
    pwm_config_set_clkdiv(&config_2, clock_get_hz(clk_sys) / (BUZZER_FREQUENCY * 4096));
    pwm_init(slice_2_num, &config_2, true);
    pwm_set_gpio_level(BUZZER_B, 0);
}

/**
 * @brief Callback para interrupções dos botões.
 * 
 * Gerencia as interrupções dos botões (BUTTON_5_PIN, BUTTON_6_PIN e SW).
 * Realiza ações como ligar/desligar o alarme, LEDs e alternar o estado da luz fixa.
 * 
 * @param gpio Número do pino que acionou a interrupção.
 * @param events Tipo de evento ocorrido (borda de subida/descida).
 */
void gpio_callback(uint gpio, uint32_t events)
{
    uint64_t current_time = time_us_64();

    if (gpio == BUTTON_5_PIN && (events & GPIO_IRQ_EDGE_FALL))
    {
        if (current_time - last_button_5_time >= DEBOUNCE_DELAY_US)
        {
            last_button_5_time = current_time;

            // Toggle do estado do alarme
            if (mic_enabled)
            {
                // Desliga se estiver ligado
                gpio_put(LED_PIN_G, 0);
                gpio_put(LED_PIN_R, 1);
                stop_beep(BUZZER_B);
                buzzer_on = false;
                mic_enabled = false;
                printf("Alarme DESLIGADO\n");
                update_display("  ALARME OFF", " ADC DISABLED");
            }
            else
            {
                // Liga se estiver desligado
                gpio_put(LED_PIN_G, 1);
                gpio_put(LED_PIN_R, 0);
                mic_enabled = true;
                listening = true;
                printf("Alarme LIGADO\n");
                update_display("  ALARME OFF", " ADC ENABLED");
            }
        }
    }

    if (gpio == BUTTON_6_PIN && (events & GPIO_IRQ_EDGE_FALL))
    {
        // Debounce: verifica se o tempo desde o último clique é maior que o delay
        if (current_time - last_button_6_time >= DEBOUNCE_DELAY_US)
        {
            last_button_6_time = current_time; // Atualiza o tempo do último clique

            // Alterna entre ligar e desligar os LEDs
            leds_enabled = !leds_enabled;

            if (leds_enabled)
            {
                printf("LEDs LIGADOS\n");
                update_display("  LEDs ON", mic_enabled ? "ADC ATIVO" : "ADC INATIVO");
            }
            else
            {
                printf("LEDs DESLIGADOS\n");
                update_display("  LEDs OFF", mic_enabled ? "ADC ATIVO" : "ADC INATIVO");
            }
        }
    }

    if (gpio == SW && (events & GPIO_IRQ_EDGE_FALL))
    {
        if (current_time - last_sw_time >= DEBOUNCE_DELAY_US)
        {
            last_sw_time = current_time;
            fixed_light = !fixed_light;
            printf("Botão SW pressionado: fixed_light = %d\n", fixed_light);
        }
    }
}

/**
 * @brief Ativa o buzzer.
 * 
 * Define o nível do PWM para gerar um som no buzzer.
 * 
 * @param pin Pino onde o buzzer está conectado.
 */
void beep(uint pin)
{
    pwm_set_gpio_level(pin, 2048);
}

/**
 * @brief Desativa o buzzer.
 * 
 * Define o nível do PWM como 0 para silenciar o buzzer.
 * 
 * @param pin Pino onde o buzzer está conectado.
 */
void stop_beep(uint pin)
{
    pwm_set_gpio_level(pin, 0);
}