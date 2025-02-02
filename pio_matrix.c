#include "pico/stdlib.h"
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/adc.h"
#include "pio_matrix.pio.h"

#define LED_RED  13 //GPIO
#define BOTAO_A 5 //GPIO
#define BOTAO_B 6 //GPIO

#define NUM_LEDS 25 //Numero de LEDs
#define MATRIZ_PIN 7 //GPIO

#define DEBOUNCE_DELAY 200 // 200 ms para debouncing

volatile int numero = 0; //Numero atual da matriz de LED
volatile int ultima_interrup = 0; // Para armazenar o último tempo de interrupção

//Mapeamento físico dos LEDs
int PHYSICAL_LEDS_MAPPER[25] = {
    24, 23, 22, 21, 20,
    15, 16, 17, 18, 19,
    14, 13, 12, 11, 10,
    5, 6, 7, 8, 9,
    4, 3, 2, 1, 0};
//Vetores dos números
double numero0[25] =
    {0.0, 1.0, 1.0, 1.0, 0.0,  // 0
     0.0, 1.0, 0.0, 1.0, 0.0,
     0.0, 1.0, 0.0, 1.0, 0.0,
     0.0, 1.0, 0.0, 1.0, 0.0,
     0.0, 1.0, 1.0, 1.0, 0.0};
double numero1[25] =
    {0.0, 0.0, 1.0, 0.0, 0.0,  // 1
     0.0, 1.0, 1.0, 0.0, 0.0,
     0.0, 0.0, 1.0, 0.0, 0.0,
     0.0, 0.0, 1.0, 0.0, 0.0,
     0.0, 1.0, 1.0, 1.0, 0.0};
double numero2[25] =
    {0.0, 1.0, 1.0, 1.0, 0.0,  // 2
     0.0, 0.0, 0.0, 1.0, 0.0,
     0.0, 1.0, 1.0, 1.0, 0.0,
     0.0, 1.0, 0.0, 0.0, 0.0,
     0.0, 1.0, 1.0, 1.0, 0.0};
double numero3[25] =
    {0.0, 1.0, 1.0, 1.0, 0.0,  // 3
     0.0, 0.0, 0.0, 1.0, 0.0,
     0.0, 1.0, 1.0, 1.0, 0.0,
     0.0, 0.0, 0.0, 1.0, 0.0,
     0.0, 1.0, 1.0, 1.0, 0.0};
double numero4[25] =
    {0.0, 1.0, 0.0, 1.0, 0.0,  // 4
     0.0, 1.0, 0.0, 1.0, 0.0,
     0.0, 1.0, 1.0, 1.0, 0.0,
     0.0, 0.0, 0.0, 1.0, 0.0,
     0.0, 0.0, 0.0, 1.0, 0.0};
double numero5[25] =
    {0.0, 1.0, 1.0, 1.0, 0.0,  // 5
     0.0, 1.0, 0.0, 0.0, 0.0,
     0.0, 1.0, 1.0, 1.0, 0.0,
     0.0, 0.0, 0.0, 1.0, 0.0,
     0.0, 1.0, 1.0, 1.0, 0.0};
double numero6[25] =
    {0.0, 1.0, 1.0, 1.0, 0.0,  // 6
     0.0, 1.0, 0.0, 0.0, 0.0,
     0.0, 1.0, 1.0, 1.0, 0.0,
     0.0, 1.0, 0.0, 1.0, 0.0,
     0.0, 1.0, 1.0, 1.0, 0.0};
double numero7[25] =
    {0.0, 1.0, 1.0, 1.0, 0.0,  // 7
     0.0, 0.0, 0.0, 1.0, 0.0,
     0.0, 0.0, 0.0, 1.0, 0.0,
     0.0, 0.0, 1.0, 0.0, 0.0,
     0.0, 0.0, 1.0, 0.0, 0.0};
double numero8[25] =
    {0.0, 1.0, 1.0, 1.0, 0.0,  // 8
     0.0, 1.0, 0.0, 1.0, 0.0,
     0.0, 1.0, 1.0, 1.0, 0.0,
     0.0, 1.0, 0.0, 1.0, 0.0,
     0.0, 1.0, 1.0, 1.0, 0.0};
double numero9[25] =
    {0.0, 1.0, 1.0, 1.0, 0.0,  // 9
     0.0, 1.0, 0.0, 1.0, 0.0,
     0.0, 1.0, 1.0, 1.0, 0.0,
     0.0, 0.0, 0.0, 1.0, 0.0,
     0.0, 1.0, 1.0, 1.0, 0.0};
//Ponteiro para os vetores dos números
double *nums[]={numero0, numero1, numero2, numero3, numero4, numero5, numero6, numero7, numero8, numero9};
//Inicialização de funções
void init_gpios();
void pisca_led();
static void gpio_irq_handler(uint gpio, uint32_t events);
void padrao(double *desenho, uint32_t valor_led, PIO pio, uint sm, double r, double g, double b);

int main() {
  PIO pio = pio0; 
  bool ok;
  uint16_t i;
  uint32_t valor_led;
  double r, g, b;

  ok = set_sys_clock_khz(128000, false);
  stdio_init_all();
  init_gpios();

  uint sm = pio_claim_unused_sm(pio, true);
  uint offset = pio_add_program(pio, &pio_matrix_program);
  pio_matrix_program_init(pio, sm, offset, MATRIZ_PIN);
  //Habilita botões de interrupção
  gpio_set_irq_enabled_with_callback(BOTAO_A, GPIO_IRQ_EDGE_FALL, 1, &gpio_irq_handler);
  gpio_set_irq_enabled_with_callback(BOTAO_B, GPIO_IRQ_EDGE_FALL, 1, &gpio_irq_handler);
  //Escolha a cor
  r = 1;
  g = 0;
  b = 0;
  //Looping
  while (true)
  {
    pisca_led();
    padrao(nums[numero], valor_led, pio, sm, r, g, b);
  }
    return 0;
}
//Inicializa GPIOS
void init_gpios(){
  gpio_init(LED_RED);
  gpio_set_dir(LED_RED, GPIO_OUT);

  gpio_init(BOTAO_A);
  gpio_set_dir(BOTAO_A, GPIO_IN);
  gpio_pull_up(BOTAO_A);

  gpio_init(BOTAO_B);
  gpio_set_dir(BOTAO_B, GPIO_IN);
  gpio_pull_up(BOTAO_B);
}
//Função para piscar LED
void pisca_led(){
  gpio_put(LED_RED, true);
  sleep_ms(100);
  gpio_put(LED_RED, false);
  sleep_ms(100);
}
//Função para conversão dos valores das cores
uint32_t matrix_rgb(double r, double g, double b){
  unsigned char R, G, B;
  R = r * 255;
  G = g * 255;
  B = b * 255;
  return (G<<24) | (R << 16) | (B << 8);
}
//Função padrão para ligar os LEDs escolhidos
void padrao(double *desenho, uint32_t valor_led, PIO pio, uint sm, double r, double g, double b){
    for (int16_t i = 0; i < NUM_LEDS; i++)
    {
        int led_matrix_location = PHYSICAL_LEDS_MAPPER[i];
        valor_led = matrix_rgb(r*desenho[led_matrix_location], g*desenho[led_matrix_location], b*desenho[led_matrix_location]);
        pio_sm_put_blocking(pio, sm, valor_led);
    }
}
//Função de ação para interrupção
static void gpio_irq_handler(uint gpio, uint32_t events){
  uint32_t tempo_interrup = to_ms_since_boot(get_absolute_time()); // Obtém o tempo atual
    if (tempo_interrup - ultima_interrup > DEBOUNCE_DELAY) { // Verifica o tempo de debounce
        ultima_interrup = tempo_interrup; // Atualiza o tempo da última interrupção
        
      if (gpio_get(BOTAO_A)==0){ //Incremento
          numero+=1;
            if (numero>9){
                numero=0;
            }
      }
      else if (gpio_get(BOTAO_B)==0){ //Decremento
               numero-=1;
            if (numero<0){
                numero=9;
            }   
      }
    }
}
