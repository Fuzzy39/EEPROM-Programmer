#pragma once
#include <libusb-1.0/libusb.h>

// nothing here...

enum DeviceSpeed
{
    HIGH = 5,       // M>>5 375khz (right now: 46.875khz)
    LOW = 10,        // M>>10 11.719khz (1465 hz right now)
};




const int CDC_INTERFACE = 0;
const int CDC_DATA_INTERFACE = 1;
const int HID_INTERFACE = 2;

// endpoints
const uint8_t CDC_ENDPOINT = 0x01;
const uint8_t UART_ENDPOINT = 0x02;
const uint8_t HID_ENDPOINT = 0x03;
const uint8_t IN = 0x80;
const uint8_t OUT = 0x00;

// ms   
const int TIMEOUT = 1000;



// basic libusb stuff
void listDevices();
void bailOnError(int error);


// handling the programmer
libusb_device* findProgrammer();
void enumerateProgrammer(libusb_device* programmer);
libusb_device_handle* openProgrammer(libusb_device * programmer);
// handle may be null.
void closeProgrammer(libusb_device* programmer, libusb_device_handle* handle);



void setSpeed(libusb_device_handle* handle, DeviceSpeed speed);

