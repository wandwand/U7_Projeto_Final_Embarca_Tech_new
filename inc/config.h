#ifndef CONFIG_H
#define CONFIG_H

//LED RGB
#define LED_PIN_G 11
#define LED_PIN_B 12
#define LED_PIN_R 13

//Variaveis joystick
#define SW 22             // Pino de leitura do botão do joystick
#define BUTTON_6_PIN 6     //Pino de leitura do botão B lado direito da placa
static bool fixed_light;    // Variável para verificar se a luz está fixa ou não
extern bool leds_enabled;

#endif