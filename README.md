## U4C4O12T - Interrupções
Neste projeto, utiliza-se os seguintes componentes conectados à placa BitDogLab:
• Matriz 5x5 de LEDs (endereçáveis) WS2812, conectada à GPIO 7.
• LED RGB, com os pinos conectados às GPIOs (11, 12 e 13).
• Botão A conectado à GPIO 5.
• Botão B conectado à GPIO 6.

# Funcionalidades do Projeto
1. O LED vermelho do LED RGB deve piscar continuamente 5 vezes por segundo.
2. O botão A deve incrementar o número exibido na matriz de LEDs cada vez que for pressionado.
3. O botão B deve decrementar o número exibido na matriz de LEDs cada vez que for pressionado.
4. Os LEDs WS2812 devem ser usados para criar efeitos visuais representando números de 0 a 9.

# Características

O programa utiliza a interrupção gpio_set_irq_enabled_with_callback para criar a rotina de troca de números exibidos na matriz de LEDs.
Para evitar leituras indesejadas foi programado um sistema de debouncing.

# Ensaio
<URL>