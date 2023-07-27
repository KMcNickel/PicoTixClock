#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/rtc.h"
#include "ws2812.pio.h"
#include "math.h"
#include "include/ds3231.h"

#define LED_COLOR_COUNT 3   //RGB LEDs
#define LED_LANE_COUNT 4    //4 strips on different pio pins
#define MAX_LED_COUNT 9     //max of 9 leds per strip (some have less)
#define LED_BASE_PIN 19
#define WS2812_FREQUENCY 800000

#define MAX_BRIGHTNESS_INPUT 255
#define MAX_BRIGHTNESS_OUTPUT 15
#define BRIGHTNESS_DIVIDEND (MAX_BRIGHTNESS_INPUT / MAX_BRIGHTNESS_OUTPUT)

#define DS3231_I2C_PERIPHERAL i2c1
#define DS3231_SDA_PIN 10
#define DS3231_SCL_PIN 11

#define TICK_TIMER_DELAY -1000

static uint32_t ledTxBuf[LED_COLOR_COUNT * MAX_LED_COUNT];
int ws2812StateMachine;

struct repeating_timer tickTimer;
uint8_t ledUpdateTickThreshold = 5;
uint8_t timeReportTickThreshold = 60;
uint8_t timeReportTicks = 0;
uint8_t ledUpdateTicks = 0;

DS3231 rtc(DS3231_I2C_PERIPHERAL);
bool use12HourTime = false;

std::string stdInBuffer;

#define COLOR_LIST_SIZE 7
uint32_t colors[] = 
{
    0x404040,
    0xFF0000,
    0x00FF00,
    0x0000FF,
    0x800080,
    0x808000,
    0x008080
};

uint8_t laneSize[] = 
{
    3,
    9,
    6,
    9
};

uint8_t scaleColor(uint8_t value)
{
    return value / BRIGHTNESS_DIVIDEND;
}

uint32_t setColorValue(uint8_t laneNum, uint32_t colorWord, uint8_t requestedValue)
{
    uint64_t mask = 0xEEEEEEEEEEEEEEEE << laneNum;
    uint32_t shiftedValue = 0;

    colorWord = colorWord & (uint32_t) (mask >> 32);

    if(requestedValue == 0) return colorWord;

    requestedValue = scaleColor(requestedValue);

    for(int i = 0; i < 8; i++)
        shiftedValue |= (requestedValue & (1 << i)) << (3 * i);

    shiftedValue = shiftedValue << laneNum;

    colorWord |= shiftedValue;

    //printf("Color Word is: 0x%08X\r\n", colorWord);

    return colorWord;
}

uint8_t setColors(uint8_t laneNum, uint8_t ledNum, uint8_t red, uint8_t green, uint8_t blue)
{
    if(ledNum > MAX_LED_COUNT) return 1;
    if(laneNum > LED_LANE_COUNT) return 1;

    //printf("LED %u.%u value is 0x%06X\r\n", laneNum, ledNum, color);

    uint8_t baseIndex = ledNum * LED_COLOR_COUNT;

    ledTxBuf[baseIndex] = setColorValue(laneNum, ledTxBuf[baseIndex], red);             //Green
    ledTxBuf[baseIndex + 1] = setColorValue(laneNum, ledTxBuf[baseIndex + 1], green);   //Red
    ledTxBuf[baseIndex + 2] = setColorValue(laneNum, ledTxBuf[baseIndex + 2], blue);    //Blue

    return 0;
}

uint8_t setColors(uint8_t laneNum, uint8_t ledNum, uint32_t color)
{
    if(color > 0x00FFFFFF) return 1;

    return setColors(laneNum, ledNum, (color & 0x00FF0000) >> 16, (color & 0x0000FF00) >> 8, (color & 0x000000FF));
}

uint8_t transferPixels(uint32_t ledNum)
{
    uint8_t dataArrayIndex = ledNum * LED_COLOR_COUNT;

    //printf("LED Num: %2u Red: 0x%08X Green: 0x%08X Blue: 0x%08X\r\n", 
    //        ledNum, ledTxBuf[dataArrayIndex + 1], ledTxBuf[dataArrayIndex + 0], ledTxBuf[dataArrayIndex + 2]);

    for(int i = 0; i < LED_COLOR_COUNT; i++)
        pio_sm_put_blocking(pio0, ws2812StateMachine, ledTxBuf[dataArrayIndex + i]);

    return 0;
}

void shuffleArray(uint8_t * array, uint8_t len)
{
    uint8_t temp;

    for (int i = len - 1; i > 0; i--)
    {
        int j = rand() % (i + 1);
        temp = *(array + i);
        *(array + i) = *(array + j);
        *(array + j) = temp;
    }
}

void shuffleArray(uint32_t * array, uint8_t len)
{
    uint32_t temp;

    for (int i = len - 1; i > 0; i--)
    {
        int j = rand() % (i + 1);
        temp = *(array + i);
        *(array + i) = *(array + j);
        *(array + j) = temp;
    }
}

void updateDisplay()
{
    datetime_t currentTime;
    uint8_t ledCount[4];
    uint8_t ledsAvailable[MAX_LED_COUNT];

    shuffleArray(colors, COLOR_LIST_SIZE);

    rtc.readTime(&currentTime);

    if(use12HourTime)
    {
        if(currentTime.hour > 12) currentTime.hour -= 12;
        if(currentTime.hour == 0) currentTime.hour = 12;
    }

    ledCount[0] = currentTime.hour / 10;
    ledCount[1] = currentTime.hour % 10;
    ledCount[2] = currentTime.min / 10;
    ledCount[3] = currentTime.min % 10;

    for(int i = 0; i < 4; i++)
    {
        for(int j = 0; j < MAX_LED_COUNT; j++)
        {
            setColors(i, j, 0x000000);
            ledsAvailable[j] = j;
        }
        if(ledCount[i] == 0) continue;
        if(ledCount[i] == 1) setColors(i, rand() % laneSize[i], colors[i]);
        else
        {
            shuffleArray(ledsAvailable, laneSize[i]);
            for(int j = 0; j < ledCount[i]; j++)
                setColors(i, ledsAvailable[j], colors[i]);
        }
    }

    for(int i = 0; i < MAX_LED_COUNT; i++)
        transferPixels(i);
}

bool tickTimerCallback(struct repeating_timer *t)
{
    timeReportTicks++;
    ledUpdateTicks++;

    if(timeReportTicks == timeReportTickThreshold)
    {
        datetime_t currentTime;

        rtc.readTime(&currentTime);
        printf("The current short time is: %02u:%02u\r\n",
            currentTime.hour, currentTime.min);
        timeReportTicks = 0;
    }

    if(ledUpdateTicks == ledUpdateTickThreshold)
    {
        updateDisplay();
        ledUpdateTicks = 0;
    }
    
    return true;
}

void processTerminalString(std::string terminalString)
{
    printf("String: %s\r\n", terminalString.c_str());
}

void charsAvailableCallback(void * param)
{
    char incoming = getchar_timeout_us(0);
    if(incoming == PICO_ERROR_TIMEOUT) return;

    if(incoming == '\r' || incoming == '\n')
    {
        if(stdInBuffer.empty()) return;
        putchar('\r');
        putchar('\n');
        processTerminalString(stdInBuffer);
        stdInBuffer.clear();
        return;
    }

    putchar_raw(incoming);
    stdInBuffer.push_back(incoming);
}

void initializeWS2812()
{
    for(int i = 0; i < LED_COLOR_COUNT * MAX_LED_COUNT; i++)
        ledTxBuf[i] = 0;

    PIO pio = pio0;
    ws2812StateMachine = 0;
    uint offset = pio_add_program(pio, &ws2812_program);

    ws2812_program_init(pio, ws2812StateMachine, offset, LED_BASE_PIN, LED_LANE_COUNT, WS2812_FREQUENCY);
}

void initializeDS3231()
{
    datetime_t timeDataSet;

    i2c_init(DS3231_I2C_PERIPHERAL, 100000);
    gpio_set_function(DS3231_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(DS3231_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(DS3231_SDA_PIN);
	gpio_pull_up(DS3231_SCL_PIN);

    rtc.readTime(&timeDataSet);
    rtc.pushTimeToPicoRTC();
    printf("The current DS3231 time is: %u-%u-%u %02u:%02u:%02u\r\n",
            timeDataSet.month, timeDataSet.day, timeDataSet.year,
            timeDataSet.hour, timeDataSet.min, timeDataSet.sec);
    /*rtc_get_datetime(&timeDataSet);
    printf("The current RP2040 time is: %u-%u-%u %02u:%02u:%02u\r\n",
            timeDataSet.month, timeDataSet.day, timeDataSet.year,
            timeDataSet.hour, timeDataSet.min, timeDataSet.sec);*/
}

void initializeTimers()
{
    add_repeating_timer_ms(TICK_TIMER_DELAY, tickTimerCallback, NULL, &tickTimer);
}

void initializeCLI()
{
    stdInBuffer.clear();
    stdio_set_chars_available_callback(charsAvailableCallback, NULL);
}

void initialize()
{
    stdio_init_all();

    printf("\r\n\r\n\r\n\r\nTix Clock Project\r\n\r\n");

    initializeWS2812();
    initializeDS3231();
    initializeTimers();
    initializeCLI();
}

int main() {
    initialize();

    while(1)
    { }
}
