/*
 * WS2812.cpp
 *
 *  Created on: 21.12.2019
 *      Author: Marcin
 */

#include "WS2812.h"

WS2812::WS2812(PinName dataPin, PinName clkPin, uint16_t numberOfDevices) :
    interface(dataPin, NC, clkPin),
    numberOfDevices(numberOfDevices)
{
    pRGBData = new uint32_t[numberOfDevices];
    for(uint8_t k=0; k<numberOfDevices; k++) { pRGBData[k] = 0; }
    interface.frequency(4000000);
    interface.format(8, 1);
}

/*
 * update RGB LED devices in the chain
 */
void WS2812::update(void)
{
    static DigitalOut testSignal(PC_9); //XXX
    testSignal = !testSignal;   //XXX

    oneWireBuffer.clear();

    // iterate through all devices
    for(uint16_t deviceIndex = 0; deviceIndex < numberOfDevices; deviceIndex++)
    {
        // iterate through 3 colors
        for(uint8_t colorIndex = 0; colorIndex < 3; colorIndex++)
        {
            uint8_t colorValue = ((*(pRGBData + deviceIndex)) >> (2-colorIndex) * 8) & 0xFF;
            // iterate through pairs of bits
            for(uint8_t pairIndex = 0; pairIndex < 4; pairIndex++)
            {
                uint8_t pairOfPulses = (colorValue & 0x80) ? bitOnePattern : bitZeroPattern;
                pairOfPulses <<= 4;
                pairOfPulses |= (colorValue & 0x40) ? bitOnePattern : bitZeroPattern;
                oneWireBuffer.push_back(pairOfPulses);
                colorValue <<= 2;
            }
        }
    }

    interface.transfer<uint8_t>(&oneWireBuffer[0], oneWireBuffer.size(), nullptr, 0, nullptr);
}
