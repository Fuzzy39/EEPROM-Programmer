#include <iostream>
#include "main.hpp"
#include "Comms.hpp"
#include <iomanip>
#include <string.h> 
#include <stdio.h>


void sendSpeeds(libusb_device_handle* handle)
{
    unsigned char byte = 0x00;
    DeviceSpeed speed = DeviceSpeed::LOW;
    while(true)
    {
       
        std::cout<<"Attempting to send 0x"<<std::hex<<(int)byte<<" at "<<(speed==LOW?"Low":"High")<<" speed.\n";

        // try to send a number?

        // set speed
        setSpeed(handle, speed);
        // send data
        bailOnError(libusb_bulk_transfer(handle, UART_ENDPOINT|USB_OUT, &byte, 1, nullptr, 3000));
        // fingers crossed!

        // change speed
        if(speed == DeviceSpeed::LOW)
        {
            //speed = DeviceSpeed::HIGH;
        }
        else
        {
            speed = DeviceSpeed::LOW;
        }

        byte++;
       
        std::cin.get();
    }

}

void read(libusb_device_handle* handle)
{
    setSpeed(handle, DeviceSpeed::HIGH);
    confirmInitialState(handle);
    for(uint16_t i= 0;  i<256; i++)
    {
        std::cout<<"Reading Addr: 0x"<<std::hex<<(int)i<<"\n";
        std::cout<<"Got: 0x"<<std::hex<<(int)readByte(handle, i)<<"\n";
    }
    
    
}

void readRom(libusb_device_handle* handle, size_t size)
{
    setSpeed(handle, DeviceSpeed::LOW);
    confirmInitialState(handle);

    uint8_t* buffer = (uint8_t*)malloc(size);
    std::string filename = "output.bin";

    for(uint16_t i= 0;  i<size; i++)
    {
        if(i%(1024/8)==0)
        {
            std::cout<<"0x"<<std::hex<<std::setw(4) << std::setfill('0')<<(int)i<<"...\n";
        }
        uint8_t data = readByte(handle, i);
        //std::cout<<"Got: 0x"<<std::hex<<(int)data<<"\n";
        buffer[i] = data;

    }

    FILE* file = fopen(filename.c_str(), "w");
    fwrite(buffer, 1, size, file);
    fclose(file);
    std::cout<<"Done! Wrote to '"<<filename.c_str()<<"'.\n";

}

void sendAndRecieve(libusb_device_handle* handle)
{
    setSpeed(handle, DeviceSpeed::LOW);

    confirmInitialState(handle);
    uint8_t toSend[] = {0, 255, 1};
    while(true)
    {
        std::cout<<"Sent 0x"<<std::hex<<(int)toSend[0]<<", 0x"<<(int)toSend[1]<<", 0x"<<(int)toSend[2]<<".\n";
        bailOnError(libusb_bulk_transfer(handle, UART_ENDPOINT|USB_OUT, &toSend[0], 3, nullptr, TIMEOUT));  
        uint8_t received = receiveByte(handle);
        
        for(int i = 0; i<3; i++)
        {
            toSend[i]++;
        }

        std::cout<<"Got 0x"<<std::hex<<(int)(received)<<" back.\n";
        std::cin.get();

    }


}

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

    //sendSpeeds(handle);
    readRom(handle, 8192);
   
    closeProgrammer(programmer, handle);
    libusb_exit(nullptr);
    
    return 0;
}



