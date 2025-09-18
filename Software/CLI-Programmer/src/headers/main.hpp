#pragma once
#include <libusb-1.0/libusb.h>

// nothing here...

//void setSpeed(libusb_device_handle* handle, ??? speed);
libusb_device* findProgrammer();
// libusb_device_handle* openProgrammer(libusb_device * programmer);

// // handle may be null.
// void closeProgrammer(libusb_device* programmer, libusb_device_handle* handle);


void enumerateProgrammer(libusb_device* programmer);
void listDevices();

void bailOnError(int error, );
