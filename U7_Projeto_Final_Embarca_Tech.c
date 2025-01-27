#include "main.h"
// Pinos e definições
#define I2C_SDA 14
#define I2C_SCL 15
#define MIC_CHANNEL 2
#define MIC_PIN (26 + MIC_CHANNEL)
#define ADC_CLOCK_DIV 96.f
#define SAMPLES 200
#define ADC_ADJUST(x) (x * 3.3f / (1 << 12u) - 1.65f)
#define BUZZER_PIN 21
#define BUZZER_FREQUENCY 100
#define BUTTON_5_PIN 5
#define BUTTON_6_PIN 6
#define DEBOUNCE_DELAY_US 50000 

// Variáveis globais
volatile bool adc_enabled = false;
volatile bool buzzer_on = false;
volatile bool listening = false;
uint dma_channel;
dma_channel_config dma_cfg;
uint16_t adc_buffer[SAMPLES];
uint8_t ssd[ssd1306_buffer_length];
uint64_t last_button_5_time = 0;
uint64_t last_button_6_time = 0;
uint64_t last_sw_time = 0;

// Estrutura para renderização no display
struct render_area frame_area = {
    .start_column = 0,
    .end_column = ssd1306_width - 1,
    .start_page = 0,
    .end_page = ssd1306_n_pages - 1};

// Protótipos
void setup_peripherals();
void setup_display();
void setup_adc();
void setup_buttons();
void setup_buzzer();
void sample_mic();
float mic_power();
uint8_t get_intensity(float v);
void beep(uint pin);
void stop_beep(uint pin);
void update_display(const char *line1, const char *line2);
void gpio_callback(uint gpio, uint32_t events);


void alarme()
{

    if (adc_enabled)
    {
        sample_mic();
        float avg = mic_power();
        avg = 2.f * fabs(ADC_ADJUST(avg));
        uint8_t intensity = get_intensity(avg);

        // Controle do buzzer
        if (intensity >= 2 && !buzzer_on)
        {
            beep(BUZZER_PIN);
            buzzer_on = true;
            listening = true;
        }

        printf("Intensidade: %d | Média: %.4f\n", intensity, avg);
    }

    sleep_ms(10);
}
void smart_home()
{
    while (true)
    {
        joystick(fixed_light);
        alarme();
    }
}
int main()
{
    stdio_init_all();

    // Configuração inicial
    setup_peripherals();
    setup_display();
    setup_adc();
    setup_buttons();
    setup_buzzer();
    setup();
    fixed_light = false;
    // Mensagem inicial no display
    update_display("  ALARME DESLIGADO", "  ADC NAO ACIONADO");

    smart_home();
}

// Configuração dos periféricos
void setup_peripherals()
{
    i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
}

// Configuração do display SSD1306
void setup_display()
{
    ssd1306_init();
    memset(ssd, 0, ssd1306_buffer_length);
    calculate_render_area_buffer_length(&frame_area);
    render_on_display(ssd, &frame_area);
}

// Configuração do ADC para o microfone
void setup_adc()
{
    adc_gpio_init(MIC_PIN);
    adc_init();
    adc_select_input(MIC_CHANNEL);
    adc_fifo_setup(true, true, 1, false, false);
    adc_set_clkdiv(ADC_CLOCK_DIV);
    adc_run(false);

    dma_channel = dma_claim_unused_channel(true);
    dma_cfg = dma_channel_get_default_config(dma_channel);
    channel_config_set_transfer_data_size(&dma_cfg, DMA_SIZE_16);
    channel_config_set_read_increment(&dma_cfg, false);
    channel_config_set_write_increment(&dma_cfg, true);
    channel_config_set_dreq(&dma_cfg, DREQ_ADC);
}

// Configuração dos botões
void setup_buttons()
{
    gpio_init(BUTTON_5_PIN);
    gpio_set_dir(BUTTON_5_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_5_PIN);

    gpio_init(BUTTON_6_PIN);
    gpio_set_dir(BUTTON_6_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_6_PIN);

    gpio_set_irq_enabled_with_callback(BUTTON_5_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled(BUTTON_6_PIN, GPIO_IRQ_EDGE_FALL, true);
}

// Configuração do buzzer
void setup_buzzer()
{
    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, clock_get_hz(clk_sys) / (BUZZER_FREQUENCY * 4096));
    pwm_init(slice_num, &config, true);
    pwm_set_gpio_level(BUZZER_PIN, 0);
}

// Callback de interrupção dos botões
void gpio_callback(uint gpio, uint32_t events) {
    uint64_t current_time = time_us_64(); // Obtém o tempo atual

    if (gpio == BUTTON_5_PIN && (events & GPIO_IRQ_EDGE_FALL)) {
        // Verifica debounce para BUTTON_5_PIN
        if (current_time - last_button_5_time >= DEBOUNCE_DELAY_US) {
            last_button_5_time = current_time; // Atualiza o tempo do último clique
            if (!adc_enabled) {
                adc_enabled = true;
                listening = true;
                printf("Sensor ligado (ADC habilitado).\n");
                update_display("  ALARME ON", " ADC ENABLED");
            }
        }
    }

    if (gpio == BUTTON_6_PIN && (events & GPIO_IRQ_EDGE_FALL)) {
        // Verifica debounce para BUTTON_6_PIN
        if (current_time - last_button_6_time >= DEBOUNCE_DELAY_US) {
            last_button_6_time = current_time; // Atualiza o tempo do último clique
            stop_beep(BUZZER_PIN);
            buzzer_on = false;
            adc_enabled = false;
            printf("Alarme desligado.\nSensor desligado (ADC desabilitado).\n");
            update_display("  ALARME OFF", "  ADC DISABLED");
        }
    }

    if (gpio == SW && (events & GPIO_IRQ_EDGE_FALL)) {
        // Verifica debounce para o botão SW
        if (current_time - last_sw_time >= DEBOUNCE_DELAY_US) {
            last_sw_time = current_time; // Atualiza o tempo do último clique
            fixed_light = !fixed_light; // Alterna entre luz fixa e controle pelo joystick
            printf("Botão SW pressionado: fixed_light = %d\n", fixed_light);
        }
    }
}

// Função para realizar leituras do microfone (ADC)
void sample_mic()
{
    
    adc_fifo_drain();
    dma_channel_configure(dma_channel, &dma_cfg,
                          adc_buffer, &(adc_hw->fifo), SAMPLES, true);
    adc_run(true);
    dma_channel_wait_for_finish_blocking(dma_channel);
    adc_run(false);
    printf("exit sample_mic\n");
}

// Calcula a potência média (RMS) do microfone
float mic_power()
{
    float avg = 0.f;
    for (uint i = 0; i < SAMPLES; ++i)
        avg += adc_buffer[i] * adc_buffer[i];
    printf("return mic_power\n");
    return sqrt(avg / SAMPLES);
}

// Retorna a intensidade do som (0 a 4)
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

// Liga o buzzer
void beep(uint pin)
{
    pwm_set_gpio_level(pin, 2048); // Duty cycle de 50%
}

// Desliga o buzzer
void stop_beep(uint pin)
{
    pwm_set_gpio_level(pin, 0);
}

// Atualiza o display com duas linhas de texto
void update_display(const char *line1, const char *line2)
{
    memset(ssd, 0, ssd1306_buffer_length);
    ssd1306_draw_string(ssd, 5, 0, (char *)line1);
    ssd1306_draw_string(ssd, 5, 8, (char *)line2);
    render_on_display(ssd, &frame_area);
}

