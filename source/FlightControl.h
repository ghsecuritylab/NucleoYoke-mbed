/*
 * FlightControl.h
 *
 *  Created on: 19.12.2019
 *      Author: Marcin
 */

#ifndef SOURCE_FLIGHTCONTROL_H_
#define SOURCE_FLIGHTCONTROL_H_

#include "Console.h"
#include "Display.h"
#include "mbed.h"
#include "drivers/USBHID.h"

class FlightControl
{
public:
    FlightControl(EventQueue& eventQueue);
    void handler(void);
    void connect(void);
private:
    void sendDataToSimulator(void);
    EventQueue& eventQueue;             // event queue for flight control events
    USBHID* pConnection{nullptr};       // pointer to USB HID object
    static const uint8_t HIDBufferLength = 64;
    static const uint16_t USB_VID = 0x0483;
    static const uint16_t USB_PID = 0x5750;
    static const uint16_t USB_VER = 0x0002;
    HID_REPORT inputReport = {.length = HIDBufferLength, .data = {0}};      // report from simulator
    HID_REPORT outputReport = {.length = HIDBufferLength, .data = {0}};     // report to simulator
    float throttleLeverPosition{0.0f};  // throttle lever calculated position <0..1>
    AnalogIn propellerPotentiometer;    // propeller pitch potentiometer (blue)
    AnalogIn mixturePotentiometer;      // mixture potentiometer (red)
};

#endif /* SOURCE_FLIGHTCONTROL_H_ */
