#pragma once
#include <libusb-1.0/libusb.h>

struct ChipSettings
{
    // lets assume high to  low bits!
    // byte 1
    bool CDCSerialEnumeration :1;
    uint8_t dontCare:5;
    uint8_t password:2;
    // byte 2
    uint8_t dontCare2:4;
    uint8_t clockDivider:4;
    //
    uint8_t DACreferenceVoltage : 2;
    uint8_t DACreference:1;
    uint8_t DACPowerUp:5;
   
    // ADC option is here but eh.
    uint8_t dontCare3;

    uint8_t VIDLow;
    uint8_t VIDHigh;

    uint8_t PIDLow;
    uint8_t PIDHigh;

    uint8_t PowerAttributes;
    uint8_t mAmpsRequested;


    ChipSettings(libusb_device_handle* handle);

    void updateSettings(libusb_device_handle* handle);
};