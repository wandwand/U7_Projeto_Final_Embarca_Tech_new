#include "main.h"
#include "inc\display.h"
#include "inc\mic.h"

#define DEBOUNCE_DELAY_US 50000

// Variáveis globais 
bool leds_enabled = true;
volatile bool mic_enabled = false;
volatile bool listening = false;

volatile bool buzzer_on = false;


uint64_t last_button_5_time = 0;
uint64_t last_button_6_time = 0;
uint64_t last_sw_time = 0;




// Função principal do alarme
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

void smart_home()
{
    while (true)
    {
        joystick(fixed_light);
        alarme();
    }
}






// Configuração dos botões
void setup_buttons()
{
    gpio_init(BUTTON_5_PIN);
    gpio_set_dir(BUTTON_5_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_5_PIN);

    gpio_set_irq_enabled_with_callback(BUTTON_5_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled(BUTTON_6_PIN, GPIO_IRQ_EDGE_FALL, true);
}

// Configuração dos LEDs RGB
void setup_led_rgb()
{
    gpio_init(LED_PIN_R);
    gpio_init(LED_PIN_B);
    gpio_init(LED_PIN_G);

    gpio_set_dir(LED_PIN_R, GPIO_OUT);
    gpio_set_dir(LED_PIN_B, GPIO_OUT);
    gpio_set_dir(LED_PIN_G, GPIO_OUT);
}

// Configuração do buzzer
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

// Callback para interrupções de GPIO
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



// Ativa o buzzer
void beep(uint pin)
{
    pwm_set_gpio_level(pin, 2048);
}

// Desativa o buzzer
void stop_beep(uint pin)
{
    pwm_set_gpio_level(pin, 0);
}

