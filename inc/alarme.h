#ifndef alarme_h
#define alarme_h

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
void alarme();
void smart_home();

#endif