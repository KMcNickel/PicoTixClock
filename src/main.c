#include <stdio.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"

#define LED_COLOR_COUNT 3   //RGB LEDs
#define LED_LANE_COUNT 4    //4 strips on different pio pins
#define MAX_LED_COUNT 9     //max of 9 leds per strip

static uint32_t ledTxBuf[MAX_LED_COUNT];
int ws2812StateMachine;

uint8_t setColor(uint8_t ledNum, uint8_t red, uint8_t green, uint8_t blue)
{
    if(ledNum > MAX_LED_COUNT) return 1;

    ledTxBuf[ledNum] = ((uint32_t)(green) << 24) | ((uint32_t)(red) << 16) | ((uint32_t)(blue) << 8);
    //printf("LED %u value is 0x%08X\r\n", ledNum, ledTxBuf[ledNum]);

    return 0;
}

uint8_t transferPixel(uint32_t ledNum)
{
    pio_sm_put_blocking(pio0, ws2812StateMachine, ledTxBuf[ledNum]);

    return 0;
}

void initialize()
{
    stdio_init_all();
    PIO pio = pio0;
    ws2812StateMachine = 0;
    uint offset = pio_add_program(pio, &ws2812_program);

    ws2812_program_init(pio, ws2812StateMachine, offset, 19, 800000);
}

int main() {
    initialize();

    setColor(0, 10, 0, 0);
    setColor(1, 0, 10, 0);
    setColor(2, 0, 0, 10);
    setColor(3, 0, 0, 0);
    setColor(4, 0, 0, 0);
    setColor(5, 0, 0, 0);
    setColor(6, 0, 0, 0);
    setColor(7, 0, 0, 0);
    setColor(8, 0, 0, 0);

    while(1)
    {
        for(int i = 0; i < MAX_LED_COUNT; i++)
            transferPixel(i);
        sleep_ms(10);
    }
}
