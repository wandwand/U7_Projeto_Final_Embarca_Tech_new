#include "main.h"

#define I2C_SDA 14
#define I2C_SCL 15
#define MIC_CHANNEL 2
#define MIC_PIN (26 + MIC_CHANNEL)
#define ADC_CLOCK_DIV 256.f  // Clock seguro (~488 kHz)
#define ADC_ADJUST(x) (x * 3.3f / (1 << 12u) - 1.65f) 
//#define BUZZER_B 10   // Buzzer B 
#define BUZZER_FREQUENCY 100
#define BUTTON_5_PIN 5
#define BUTTON_6_PIN 6
#define DEBOUNCE_DELAY_US 50000 

// Variáveis globais (definidas aqui)
bool leds_enabled = true;
volatile bool adc_enabled = false;
volatile bool buzzer_on = false;
volatile bool listening = false;
uint16_t adc_buffer[SAMPLES];
uint8_t ssd[ssd1306_buffer_length];
uint64_t last_button_5_time = 0;
uint64_t last_button_6_time = 0;
uint64_t last_sw_time = 0;

struct render_area frame_area = {
    .start_column = 0,
    .end_column = ssd1306_width - 1,
    .start_page = 0,
    .end_page = ssd1306_n_pages - 1};

// Implementação das funções
void alarme() {
    if (adc_enabled) {
        sample_mic();
        float avg = mic_power();
        avg = 2.f * fabs(ADC_ADJUST(avg));
        uint8_t intensity = get_intensity(avg);

        if (intensity >= 2 && !buzzer_on) {
            beep(BUZZER_B);
            buzzer_on = true;
            listening = true;
            update_display("  ALARME ON", " ADC ENABLED");
            printf("Intensidade crítica: %d (Alarme disparado)\n", intensity); // Aviso único
        }
        else if (intensity <= 2) { // Só imprime intensidades normais
            printf("Intensidade: %d\n", intensity);
        }

        if (buzzer_on){
            printf("Alarme em disparo \n");
        }
    }
    //sleep_ms(10);
}

void smart_home() {
    while (true) {
        joystick(fixed_light);
        alarme();
    }
}

void setup_peripherals() {
    i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
}

void setup_display() {
    ssd1306_init();
    memset(ssd, 0, ssd1306_buffer_length);
    calculate_render_area_buffer_length(&frame_area);
    render_on_display(ssd, &frame_area);
}

void setup_adc() {
    adc_init(); //Testando alarme sozinho

    adc_gpio_init(MIC_PIN);  // ADC já inicializado pelo joystick
    adc_select_input(MIC_CHANNEL);
    adc_set_clkdiv(ADC_CLOCK_DIV);  // Clock ajustado
}

void setup_buttons() {
    gpio_init(BUTTON_5_PIN);
    gpio_set_dir(BUTTON_5_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_5_PIN);

    gpio_init(BUTTON_6_PIN);
    gpio_set_dir(BUTTON_6_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_6_PIN);

    gpio_set_irq_enabled_with_callback(BUTTON_5_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled(BUTTON_6_PIN, GPIO_IRQ_EDGE_FALL, true);
}

void setup_buzzer() {

    //Buzzer B
    gpio_init(BUZZER_B);
    gpio_set_function(BUZZER_B, GPIO_FUNC_PWM);
    uint slice_2_num = pwm_gpio_to_slice_num(BUZZER_B);
    pwm_config config_2 = pwm_get_default_config();
    pwm_config_set_clkdiv(&config_2, clock_get_hz(clk_sys) / (BUZZER_FREQUENCY * 4096));
    pwm_init(slice_2_num, &config_2, true);
    pwm_set_gpio_level(BUZZER_B, 0);
}

void gpio_callback(uint gpio, uint32_t events) {
    uint64_t current_time = time_us_64();

    if (gpio == BUTTON_5_PIN && (events & GPIO_IRQ_EDGE_FALL)) {
        if (current_time - last_button_5_time >= DEBOUNCE_DELAY_US) {
            last_button_5_time = current_time;
            
            // Toggle do estado do alarme
            if (adc_enabled) {
                // Desliga se estiver ligado
                stop_beep(BUZZER_B);
                buzzer_on = false;
                adc_enabled = false;
                printf("Alarme DESLIGADO\n");
                update_display("  ALARME OFF", " ADC DISABLED");
            } else {
                // Liga se estiver desligado
                adc_enabled = true;
                listening = true;
                printf("Alarme LIGADO\n");
                update_display("  ALARME OFF", " ADC ENABLED");
            }
        }
    }

    if (gpio == BUTTON_6_PIN && (events & GPIO_IRQ_EDGE_FALL)) {
        // Debounce: verifica se o tempo desde o último clique é maior que o delay
        if (current_time - last_button_6_time >= DEBOUNCE_DELAY_US) {
            last_button_6_time = current_time; // Atualiza o tempo do último clique

            // Alterna entre ligar e desligar os LEDs
            leds_enabled = !leds_enabled;
            
            if (leds_enabled) {
                printf("LEDs LIGADOS\n");
                update_display("  LEDs ON", adc_enabled ? "ADC ATIVO" : "ADC INATIVO");
            } else {
                printf("LEDs DESLIGADOS\n");
                update_display("  LEDs OFF", adc_enabled ? "ADC ATIVO" : "ADC INATIVO");
            }
        }
    }

    if (gpio == SW && (events & GPIO_IRQ_EDGE_FALL)) {
        if (current_time - last_sw_time >= DEBOUNCE_DELAY_US) {
            last_sw_time = current_time;
            fixed_light = !fixed_light;
            printf("Botão SW pressionado: fixed_light = %d\n", fixed_light);
        }
    }
}

void sample_mic() {
    for (uint i = 0; i < SAMPLES; ++i) {
        adc_select_input(MIC_CHANNEL);  // Define canal explicitamente
        sleep_us(2);
        adc_buffer[i] = adc_read();
    }
}

float mic_power() {
    float avg = 0.f;
    for (uint i = 0; i < SAMPLES; ++i)
        avg += adc_buffer[i] * adc_buffer[i];
    return sqrt(avg / SAMPLES);
}

uint8_t get_intensity(float v) {
    if (v < 0.2f) return 0;
    if (v < 0.4f) return 1;
    if (v < 0.6f) return 2;
    if (v < 0.8f) return 3;
    return 4;
}

void beep(uint pin) {
    pwm_set_gpio_level(pin, 2048);
    
}

void stop_beep(uint pin) {
    pwm_set_gpio_level(pin, 0);
}

void update_display(const char *line1, const char *line2) {
    memset(ssd, 0, ssd1306_buffer_length);
    ssd1306_draw_string(ssd, 5, 0, (char *)line1);
    ssd1306_draw_string(ssd, 5, 8, (char *)line2);
    render_on_display(ssd, &frame_area);
}