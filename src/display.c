#include "inc\display.h"

uint16_t adc_buffer[SAMPLES];  // Definição real
uint8_t ssd[ssd1306_buffer_length];  // Definição real

struct render_area frame_area = {
    .start_column = 0,
    .end_column = ssd1306_width - 1,
    .start_page = 0,
    .end_page = ssd1306_n_pages - 1};


/*
 * @brief Configura a interface I2C para comunicação com periféricos.
 * 
 * Inicializa a interface I2C no barramento `i2c1`, define as funções dos pinos
 * SDA e SCL, e habilita os resistores de pull-up para garantir a comunicação.
 */
void setup_peripherals()
{
    i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
}

/*
 * @brief Inicializa e configura o display OLED.
 * 
 * Esta função inicializa o controlador SSD1306, limpa o buffer do display,
 * calcula a área de renderização e atualiza o display com os novos dados.
 */
void setup_display()
{
    ssd1306_init();
    memset(ssd, 0, ssd1306_buffer_length);
    calculate_render_area_buffer_length(&frame_area);
    render_on_display(ssd, &frame_area);
}


/*
 * @brief Atualiza o display OLED com duas linhas de texto.
 * 
 * Esta função limpa o buffer do display OLED, escreve as duas linhas de texto 
 * fornecidas e atualiza o display para exibir o conteúdo.
 * 
 * @param line1 Ponteiro para a string da primeira linha.
 * @param line2 Ponteiro para a string da segunda linha.
 */
void update_display(const char *line1, const char *line2)
{
    memset(ssd, 0, ssd1306_buffer_length);
    ssd1306_draw_string(ssd, 5, 0, (char *)line1);
    ssd1306_draw_string(ssd, 5, 8, (char *)line2);
    render_on_display(ssd, &frame_area);
}
