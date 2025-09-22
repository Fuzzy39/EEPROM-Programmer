#include <iostream>
#include "chipSettings.hpp"
#include "main.hpp"
#include <iomanip>
#include <string.h> 
#include <cmath>


// this code is wrong probably, don't use it probably.

ChipSettings::ChipSettings(libusb_device_handle* handle)
{
    // in order to initialize us we need to read from usb hid.
    size_t commandSize = 0x40;
    u_int8_t * command = (u_int8_t*)calloc(commandSize, sizeof(u_int8_t));
    command[0]=0xB0; // Read Flash data
    command[1]=0x00; // Read chip settings

    // okay, send command now.
    int transfered;
    bailOnError(libusb_interrupt_transfer(handle, HID_ENDPOINT|OUT, command, commandSize, &transfered, TIMEOUT));
    if(transfered!=commandSize)
    {
        std::cout<<"ChipSettings Command transfer failed.\n";
        std::exit(100);
    }
    // get the response back
    bailOnError(libusb_interrupt_transfer(handle, HID_ENDPOINT|IN, command, commandSize, &transfered, TIMEOUT));
    if(command[1] || transfered != commandSize )
    {
        std::cout<<"There was an error getting the chip settings.\n";
        std::exit(101);
    }

    // this is a highly suspect thing to do.
    memcpy(this, command+4, sizeof(ChipSettings));
    free(command);
}


 void ChipSettings::updateSettings(libusb_device_handle* handle)
 {
    // okay, write chip settings to the device.
    // hopefully this won't brick it.
    std::cout<<"updating setings\n";
    size_t commandSize = 0x40;
    u_int8_t * command = (u_int8_t*)calloc(commandSize, sizeof(u_int8_t));
    command[0]=0xB1; // write Flash data
    command[1]=0x00; // write chip settings
    memcpy(command+2, this, sizeof(ChipSettings));
    int transfered;
    bailOnError(libusb_interrupt_transfer(handle, HID_ENDPOINT|OUT, command, commandSize, &transfered, TIMEOUT));
    if(transfered!=commandSize)
    {
        std::cout<<"UpdateSettings Command transfer failed.\n";
        std::exit(100);
    }

    bailOnError(libusb_interrupt_transfer(handle, HID_ENDPOINT|IN, command, commandSize, &transfered, TIMEOUT));
    if(command[1] || transfered != commandSize )
    {
        std::cout<<"There was an error setting the chip settings.\n";
        std::exit(101);
    }
    

  
 }