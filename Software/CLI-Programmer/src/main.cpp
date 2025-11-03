#include <iostream>
#include "main.hpp"
#include <iomanip>
#include <string.h> 

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
        bailOnError(libusb_bulk_transfer(handle, UART_ENDPOINT|OUT, &byte, 1, nullptr, 3000));
        // fingers crossed!

        // change speed
        if(speed == DeviceSpeed::LOW)
        {
         //   speed = DeviceSpeed::HIGH;
        }
        else
        {
            speed = DeviceSpeed::LOW;
        }

        byte++;
       
        std::cin.get();
    }

}

void sendAndRecieve(libusb_device_handle* handle)
{
    setSpeed(handle, DeviceSpeed::LOW);
    uint8_t byte = 0;
    while(true)
    {
       
        std::cout<<"Sent 0x"<<std::hex<<(int)byte<<", 0x"<<byte+1<<", 0x"<<byte+2<<".\n";

        // try to send a number?
        bailOnError(libusb_bulk_transfer(handle, UART_ENDPOINT|OUT, &byte, 1, nullptr, 3000));
        std::cout<<(int)byte<<"\n";
       // std::cin.get();
        byte++;
        bailOnError(libusb_bulk_transfer(handle, UART_ENDPOINT|OUT, &byte, 1, nullptr, 3000));
        std::cout<<(int)byte<<"\n";
      //  std::cin.get();
        byte++;
        bailOnError(libusb_bulk_transfer(handle, UART_ENDPOINT|OUT, &byte, 1, nullptr, 3000));
        std::cout<<(int)byte<<"\n";
        byte++;

        // now recive a byte
        uint8_t recieved;
        int transfered;
        bailOnError(libusb_bulk_transfer(handle, UART_ENDPOINT|IN, &recieved, 1, &transfered, 3000));
        if(transfered!=1)
        {
            std::cout<<"No data...\n";
        }
        else
        {
            std::cout<<"Got 0x"<<std::hex<<(int)recieved<<" back.\n";
        }
        
       
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

    sendAndRecieve(handle);
   

    closeProgrammer(programmer, handle);
    libusb_exit(nullptr);
    return 0;
}






