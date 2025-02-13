#include "inc\display.h"
uint16_t adc_buffer[SAMPLES];  // Definição real
uint8_t ssd[ssd1306_buffer_length];  // Definição real

struct render_area frame_area = {
    .start_column = 0,
    .end_column = ssd1306_width - 1,
    .start_page = 0,
    .end_page = ssd1306_n_pages - 1};


// Configuração I2C periféricos
void setup_peripherals()
{
    i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
}

// Configuração do display OLED
void setup_display()
{
    ssd1306_init();
    memset(ssd, 0, ssd1306_buffer_length);
    calculate_render_area_buffer_length(&frame_area);
    render_on_display(ssd, &frame_area);
}

// Atualiza o display OLED
void update_display(const char *line1, const char *line2)
{
    memset(ssd, 0, ssd1306_buffer_length);
    ssd1306_draw_string(ssd, 5, 0, (char *)line1);
    ssd1306_draw_string(ssd, 5, 8, (char *)line2);
    render_on_display(ssd, &frame_area);
}