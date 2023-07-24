#include <stdio.h>
#include <stdlib.h>

#include "hardware/i2c.h"
#include "hardware/rtc.h"

class DS3231
{
    public:

        uint8_t readTime(datetime_t * data);

        uint8_t writeTime(datetime_t * data);

        uint8_t pushTimeToPicoRTC();

        DS3231(i2c_inst_t * i2c);

    private:
        i2c_inst_t * peripheral;

        uint8_t bcdToBinary(uint8_t bcdVal);
        uint8_t binaryToBcd(uint8_t binVal);
};