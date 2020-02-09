/*
 * FlightControl.cpp
 *
 *  Created on: 19.12.2019
 *      Author: Marcin
 */

#include "main.h"
#include "FlightControl.h"
#include "platform/mbed_debug.h"

FlightControl::FlightControl(EventQueue& eventQueue) :
    eventQueue(eventQueue),
    propellerPotentiometer(PC_1),
    mixturePotentiometer(PC_0),
    imuInterruptSignal(USER_BUTTON)
{
    imuInterruptSignal.rise(callback(this, &FlightControl::imuInterruptHandler));
}

/*
 * handler to be called from IMU sensor interrupt routine
 * it calculates control data and send them to PC
 */
void FlightControl::handler(void)
{
    static DigitalOut redLed(LED3);
    redLed = !redLed;
}

/*
 * connect to simulator using USB HID connection
 */
void FlightControl::connect(void)
{
    // create and start USB HID device object in a disconnected state
    //pConnection = new USBHID(false, HIDBufferLength, HIDBufferLength, USB_VID, USB_PID, USB_VER);
    // create and start USB HID Joystick device object in a disconnected state
    pJoystick = new USBJoystick(USB_VID, USB_PID, USB_VER);
    // start joystick connection process
    pJoystick->connect();
    debug("Connecting to PC using USB HID joystick (VID=%#06X PID=%#06X ver=%d)\r\n", USB_VID, USB_PID, USB_VER);
}

/*
 * prepares data in the output report and sends the report to simulator
 */
void FlightControl::sendDataToSimulator(void)
{
//    outputReport.data[0] = 0;
//
//    // bytes 4-7 is the bitfield data register (buttons, switches, encoders)
//    uint32_t buttons = 0;
//    buttons |= (0 << 0);    // bit 0 - flaps up (one shot switch)
//    buttons |= (0 << 1);  // bit 1 - flaps down (one shot switch)
//    buttons |= (0 << 2);  // bit 2 - gear up (one shot switch)
//    buttons |= (0 << 3);  // bit 3 - gear down (one shot switch)
//    buttons |= (0 << 4);  // bit 4 - center pilot's view (analog joystick pushbutton) (one shot switch)
//    buttons |= (1 << 5);  // bit 5 - elevator trim up button, 0=active
//    buttons |= (1 << 6);  // bit 6 - elevator trim down button, 0=active
//    memcpy(outputReport.data+4, &buttons, sizeof(buttons));
//
//    float fParameter;
//    // bytes 8-11 for yoke pitch
//    fParameter = 0.0f;
//    memcpy(outputReport.data+8, &fParameter, sizeof(fParameter));
//    // bytes 12-15 for yoke roll
//    fParameter = 0.0f;
//    memcpy(outputReport.data+12, &fParameter, sizeof(fParameter));
//    // bytes 16-19 for rudder control
//    fParameter = 0.2f;
//    memcpy(outputReport.data+16, &fParameter, sizeof(fParameter));
//    // bytes 24-27 for mixture control
//    fParameter = 1.0f;
//    memcpy(outputReport.data+24, &fParameter, sizeof(fParameter));
//    // bytes 28-31 for propeller control
//    fParameter = 1.0f;
//    memcpy(outputReport.data+28, &fParameter, sizeof(fParameter));
//    // bytes 32-35 for analog joystick Y (pilot's view yaw)
//    fParameter = 0.0f;
//    memcpy(outputReport.data+32, &fParameter, sizeof(fParameter));
//    // bytes 36-39 for analog joystick X (pilot's view pitch)
//    fParameter = 0.0f;
//    memcpy(outputReport.data+36, &fParameter, sizeof(fParameter));
//
//    pConnection->send_nb(&outputReport);
}

/*
 * IMU sensor interrupt handler
 */
void FlightControl::imuInterruptHandler(void)
{
    eventQueue.call(callback(this, &FlightControl::handler));
}
