#include <iostream>
#include "main.hpp"
#include <iomanip>
#include <string.h> 
#include <cmath>


int main(void)
{
    libusb_init_option options[] =  {{LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_INFO}};

    if(libusb_init_context(nullptr, options,1))
    {
        std::cout<<"Failed to Initialize.\n";
        return 1;
    }
    
    listDevices();
    libusb_device* programmer = findProgrammer();
    if(programmer == nullptr)
    {
        std::cout << "Couldn't find programmer.\n";
        libusb_exit(nullptr);
        std::exit(0);
    }
  
    enumerateProgrammer(programmer);
    libusb_device_handle* handle = openProgrammer(programmer);

    
    // So, here's my understanding. endpoint 0 is an interrupt endpoint, we shove our baud speed in there.
    
    int baud = 1465;
    unsigned char data[] = {0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x08}; // according to page 32 in the usb pstn doc, this will set line coding.

    memcpy(data, &baud, sizeof(baud)); // put the baud rate in the buffer (this is really C code, isn't it)
    int transfered = 0;
    //bailOnError(libusb_interrupt_transfer(handle, 0x02, data, 8, &transfered, 0));
    transfered = libusb_control_transfer(handle, 0b00100001, 0x20, 0, 1, data, 7, 1000);
    if(transfered!= 7)
    {
        std::cout<<"Didn't transfer all of the data I guess? transfer returned:"<<transfered<<"\n";
    }
    

    
    unsigned char byte = 0x00;
    DeviceSpeed speed = DeviceSpeed::LOW;
    while(true)
    {
       
        std::cout<<"Attempting to send 0x"<<std::hex<<(int)byte<<" at "<<(speed==LOW?"Low":"High")<<" speed.\n";

        // try to send a number?

        // set speed
        setSpeed(handle, speed);
        // send data
        bailOnError(libusb_bulk_transfer(handle, 0x02, &byte, 1, nullptr, 3000));
        // fingers crossed!

        // change speed
        if(speed == DeviceSpeed::LOW)
        {
            speed = DeviceSpeed::HIGH;
        }
        else
        {
            speed = DeviceSpeed::LOW;
        }

        byte++;
       
        std::cin.get();
    }


    closeProgrammer(programmer, handle);
    libusb_exit(nullptr);
    return 0;
}



void setSpeed(libusb_device_handle* handle, DeviceSpeed speed)
{
    // baud, fortunately, isn't too bad to calculate.
    const int MAX_CLOCK = 12000000; // 12 mhz
    int baud = MAX_CLOCK>>speed>>3; // the 2 is there because of the current placement of the jumper wire.
    std::cout<<"Setting speed to "<<std::dec<<baud<<" baud\n";

    unsigned char data[] = {0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x08}; // according to page 32 in the usb pstn doc, this will set line coding.
    memcpy(data, &baud, sizeof(baud)); // put the baud rate in the buffer (this is really C code, isn't it)


    // SET_LINE_CODING 
    int transfered = libusb_control_transfer(handle, 0b00100001, 0x20, 0, CDC_ENDPOINT, data, sizeof(data), TIMEOUT);
    if(transfered!= sizeof(data))
    {
        std::cout<<"Didn't transfer all of the data I guess? transfer returned:"<<transfered<<"\n";
    }


    // now we update the clock settings in the hid device.
    int shiftsToMCP = 0;//speed-5+2; //the counter divides by 32 and the mcp expects values shifted from 48 mhz (double check!)
    ChipSettings settings = ChipSettings(handle);
    settings.clockDivider = shiftsToMCP;
    settings.updateSettings(handle);

    
}


