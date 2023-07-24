#include <stdio.h>
#include <stdlib.h>

#include "hardware/i2c.h"
#include "include/ds3231.h"
#include "hardware/rtc.h"

#define REGISTER_ADDRESS_CLOCK_Second       0x00
#define REGISTER_ADDRESS_CLOCK_Minute       0x01
#define REGISTER_ADDRESS_CLOCK_Hour         0x02
#define REGISTER_ADDRESS_CLOCK_DayOfWeek    0x03
#define REGISTER_ADDRESS_CLOCK_DayOfMonth   0x04
#define REGISTER_ADDRESS_CLOCK_Month        0x05
#define REGISTER_ADDRESS_CLOCK_Year         0x06

#define REGISTER_ADDRESS_ALARM1_Second      0x07
#define REGISTER_ADDRESS_ALARM1_Minute      0x08
#define REGISTER_ADDRESS_ALARM1_Hour        0x09
#define REGISTER_ADDRESS_ALARM1_Day         0x0A

#define REGISTER_ADDRESS_ALARM2_Minute      0x0B
#define REGISTER_ADDRESS_ALARM2_Hour        0x0C
#define REGISTER_ADDRESS_ALARM2_Day         0x0D

#define REGISTER_ADDRESS_CONTROL            0x0E
#define REGISTER_ADDRESS_CONTROL_STATUS     0x0F
#define REGISTER_ADDRESS_AGING_OFFSET       0x10
#define REGISTER_ADDRESS_TEMPERATURE_MSB    0x11
#define REGISTER_ADDRESS_TEMPERATURE_LSB    0x12

#define DS3231_ADDRESS 0x68

uint8_t DS3231::readTime(datetime_t * data)
{
    uint8_t val = REGISTER_ADDRESS_CLOCK_Second;
    uint8_t buffer[7];

    i2c_write_blocking(peripheral, DS3231_ADDRESS, &val, 1, true);
    i2c_read_blocking(peripheral, DS3231_ADDRESS, buffer, 7, false);

    data->sec = bcdToBinary(buffer[0] & 0x7F);
    data->min = bcdToBinary(buffer[1] & 0x7F);
    data->hour = bcdToBinary(buffer[2] & 0x3F);
    data->dotw = buffer[3] & 0x07;
    data->day = bcdToBinary(buffer[4] & 0x3F);
    data->month = bcdToBinary(buffer[5] & 0x1F);
    data->year = 2000 + bcdToBinary(buffer[6] & 0x7F);

    return 0;
}

uint8_t DS3231::writeTime(datetime_t * data)
{
    uint8_t buffer[8];

    buffer[0] = REGISTER_ADDRESS_CLOCK_Second;
    buffer[1] = binaryToBcd(data->sec);
    buffer[2] = binaryToBcd(data->min);
    buffer[3] = binaryToBcd(data->hour);
    buffer[4] = data->dotw;
    buffer[5] = binaryToBcd(data->day);
    buffer[6] = binaryToBcd(data->month);
    buffer[7] = binaryToBcd(data->year % 100);

    i2c_write_blocking(peripheral, DS3231_ADDRESS, buffer, 8, false);

    return 0;
}

uint8_t DS3231::pushTimeToPicoRTC()
{
    datetime_t currentTime;

    readTime(&currentTime);

    rtc_set_datetime(&currentTime);

    return 0;
}

uint8_t DS3231::bcdToBinary(uint8_t bcdVal)
{
    uint8_t binVal;

    binVal = bcdVal & 0x0F;
    binVal += ((bcdVal & 0xF0) >> 4) * 10;

    //printf("BCD: 0x%02X to Binary: %u\r\n", bcdVal, binVal);

    return binVal;
}

uint8_t DS3231::binaryToBcd(uint8_t binVal)
{
    if(binVal > 99) return 0xFF;
    uint8_t bcdVal;

    bcdVal = binVal % 10;
    bcdVal |= (binVal / 10) << 4;

    //printf("Binary: %u to BCD: 0x%02X\r\n", binVal, bcdVal);

    return bcdVal;
}

DS3231::DS3231(i2c_inst_t * i2c)
{
    peripheral = i2c;
}
