/*
 * WS2812.h
 *
 *  Created on: 21.12.2019
 *      Author: Marcin
 */

#ifndef SOURCE_WS2812_H_
#define SOURCE_WS2812_H_

#include "mbed.h"
#include <vector>

class WS2812
{
public:
    WS2812(PinName dataPin, PinName clkPin, uint16_t numberOfDevices);
    void update(void);
private:
    SPI interface;
    uint16_t numberOfDevices;       // number of devices in the chain
    uint32_t* pRGBData;                     // pointer to array of device RGB values
    std::vector<uint8_t> oneWireBuffer;     // vector of pulses being sent to devices
    const uint8_t bitOnePattern = 0b1110;   // pattern of the 1-wire bit '1'
    const uint8_t bitZeroPattern = 0b1000;   // pattern of the 1-wire bit '0'
};

#endif /* SOURCE_WS2812_H_ */
