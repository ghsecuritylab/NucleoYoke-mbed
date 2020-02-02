/*
 * FlightControl.cpp
 *
 *  Created on: 19.12.2019
 *      Author: Marcin
 */

#include "main.h"
#include "FlightControl.h"
#include "platform/mbed_debug.h"

//XXX global variables for testing
float g_leverForce;
float g_frictionForce;
float g_totalForce;
float g_leverSpeed;
float g_leverPosition;

FlightControl::FlightControl(EventQueue& eventQueue) :
    eventQueue(eventQueue),
    propellerPotentiometer(PC_1),
    mixturePotentiometer(PC_0)
{
    controlTimer.start();
}

/*
 * handler to be called periodically
 * nominal call period 10 ms
 */
void FlightControl::handler(void)
{
    static DigitalOut testSignal(PC_8); //XXX
    testSignal = 1; //XXX

    // check data received from simulator
    if((pConnection != nullptr) &&
       (pConnection->read_nb(&inputReport)))
    {
        // new data from simulator has been received
        newDataReceived = true;
        parseReceivedData();
    }

    // set all mechanical controls
    setControls();

    // send output report to simulator
    if(newDataReceived)
    {
        sendDataToSimulator();
    }

    newDataReceived = false;
    testSignal = 0; //XXX
}

/*
 * connect to simulator using USB HID connection
 */
void FlightControl::connect(void)
{
    // create and start USB HID device object in a disconnected state
    pConnection = new USBHID(false, HIDBufferLength, HIDBufferLength, USB_VID, USB_PID, USB_VER);
    // start connection process
    pConnection->connect();
    debug("Connecting to PC using USB HID (VID=%#06X PID=%#06X ver=%d)\r\n", USB_VID, USB_PID, USB_VER);
}

/*
 * parse received simulator data into simulator data structure
 */
void FlightControl::parseReceivedData(void)
{
    simulatorData.booleanFlags = *(inputReport.data+1);
    simulatorData.gearDeflection[0] = ((*(inputReport.data+3)) >> 0) & 0x03;
    simulatorData.gearDeflection[1] = ((*(inputReport.data+3)) >> 2) & 0x03;
    simulatorData.gearDeflection[2] = ((*(inputReport.data+3)) >> 4) & 0x03;
    simulatorData.flapsDeflection = *reinterpret_cast<float*>(inputReport.data+4);
    simulatorData.totalPitch = *reinterpret_cast<float*>(inputReport.data+8);
    simulatorData.totalRoll = *reinterpret_cast<float*>(inputReport.data+12);
    simulatorData.totalYaw = *reinterpret_cast<float*>(inputReport.data+16);
    simulatorData.throttle = *reinterpret_cast<float*>(inputReport.data+20);
    simulatorData.airSpeed = *reinterpret_cast<float*>(inputReport.data+24);
    simulatorData.propellerSpeed = *reinterpret_cast<float*>(inputReport.data+28);
}

/*
 * prepares data in the output report and sends the report to simulator
 */
void FlightControl::sendDataToSimulator(void)
{
    outputReport.data[0] = 0;

    // bytes 4-7 is the bitfield data register (buttons, switches, encoders)
    uint32_t buttons = 0;
    buttons |= (0 << 0);    // bit 0 - flaps up (one shot switch)
    buttons |= (0 << 1);  // bit 1 - flaps down (one shot switch)
    buttons |= (0 << 2);  // bit 2 - gear up (one shot switch)
    buttons |= (0 << 3);  // bit 3 - gear down (one shot switch)
    buttons |= (0 << 4);  // bit 4 - center pilot's view (analog joystick pushbutton) (one shot switch)
    buttons |= (1 << 5);  // bit 5 - elevator trim up button, 0=active
    buttons |= (1 << 6);  // bit 6 - elevator trim down button, 0=active
    memcpy(outputReport.data+4, &buttons, sizeof(buttons));

    float fParameter;
    // bytes 8-11 for yoke pitch
    fParameter = 0.0f;
    memcpy(outputReport.data+8, &fParameter, sizeof(fParameter));
    // bytes 12-15 for yoke roll
    fParameter = 0.0f;
    memcpy(outputReport.data+12, &fParameter, sizeof(fParameter));
    // bytes 16-19 for rudder control
    fParameter = 0.2f;
    memcpy(outputReport.data+16, &fParameter, sizeof(fParameter));
    // bytes 20-23 for throttle control
    fParameter = throttleLeverPosition;
    memcpy(outputReport.data+20, &fParameter, sizeof(fParameter));
    // bytes 24-27 for mixture control
    fParameter = 1.0f;
    memcpy(outputReport.data+24, &fParameter, sizeof(fParameter));
    // bytes 28-31 for propeller control
    fParameter = 1.0f;
    memcpy(outputReport.data+28, &fParameter, sizeof(fParameter));
    // bytes 32-35 for analog joystick Y (pilot's view yaw)
    fParameter = 0.0f;
    memcpy(outputReport.data+32, &fParameter, sizeof(fParameter));
    // bytes 36-39 for analog joystick X (pilot's view pitch)
    fParameter = 0.0f;
    memcpy(outputReport.data+36, &fParameter, sizeof(fParameter));

    pConnection->send_nb(&outputReport);
}

/*
 * set all servo according to current mode and user input
 */
void FlightControl::setControls(void)
{
    float timeElapsed = controlTimer.read();
    controlTimer.reset();

    // throttle lever calculations
    float throttleLeverUserForce = 0.0f;
    g_leverForce = throttleLeverUserForce;  //XXX
    float throttleLeverFrictionForce = ThrottleLeverFrictionCoefficient * (throttleLeverSpeed >= 0.0f ? sqrt(throttleLeverSpeed) : -sqrt(-throttleLeverSpeed));
    g_frictionForce = throttleLeverFrictionForce; //XXX
    float totalForce = throttleLeverUserForce - throttleLeverFrictionForce;
    g_totalForce = totalForce; //XXX
    throttleLeverSpeed += ThrottleLeverSpeedCoefficient * totalForce * timeElapsed;
    g_leverSpeed = simulatorData.throttle;  //XXX
    float alpha = 0.0f;
    // complementary filter for throttle lever position
    throttleLeverPosition = (1.0f - alpha) * (throttleLeverPosition + throttleLeverSpeed * timeElapsed) + alpha * simulatorData.throttle;
    if(throttleLeverPosition > 1.0f)
    {
        throttleLeverPosition = 1.0f;
        throttleLeverSpeed = 0.0f;
    }
    else if(throttleLeverPosition < 0.0f)
    {
        throttleLeverPosition = 0.0f;
        throttleLeverSpeed = 0.0f;
    }
    g_leverPosition = throttleLeverPosition; //XXX
}
