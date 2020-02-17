/*
 * I2CDevice.h
 *
 *  Created on: 17.02.2020
 *      Author: Marcin
 */

#ifndef SOURCE_I2CDEVICE_H_
#define SOURCE_I2CDEVICE_H_

#include "mbed.h"

class I2CDevice
{
public:
    I2CDevice(I2C& bus, uint8_t deviceAddress);
private:
    I2C& bus;
    uint8_t address;
};

#endif /* SOURCE_I2CDEVICE_H_ */