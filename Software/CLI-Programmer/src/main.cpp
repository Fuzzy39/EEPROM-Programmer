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

void recieveByte(libusb_device_handle* handle)
{
    // now recive a byte
    uint8_t received[2];
    int transfered = 0;
    int tries = 0;
    
    while(transfered==0)
    {
        bailOnError(libusb_bulk_transfer(handle, UART_ENDPOINT|IN, &received[0], 2, &transfered, 3000));
        tries++;
        if(tries>=10)
        {
            std::cout<<"No data...\n";
            std::cin.get();
            return;
        }
    
    }
    
    std::cout<<"Got 0x"<<std::hex<<(int)(*received)<<" back. ("<<transfered<<" bytes)\n";
    std::cin.get();

}

void sendAndRecieve(libusb_device_handle* handle)
{
    setSpeed(handle, DeviceSpeed::HIGH);

    // consume any buffered input so we start off on the right foot.
    int transfered =1;
     uint8_t received;
    while(!libusb_bulk_transfer(handle, UART_ENDPOINT|IN, &received, 1, &transfered, 50))
    {
        std::cout<<"Woop!\n";
    }


    uint8_t byte[] = {0, 255, 1};
    while(true)
    {
       
        std::cout<<"Sent 0x"<<std::hex<<(int)byte[0]<<", 0x"<<(int)byte[1]<<", 0x"<<(int)byte[2]<<".\n";

       
        // Send it all at once?
        bailOnError(libusb_bulk_transfer(handle, UART_ENDPOINT|OUT, &byte[0], 3, nullptr, 3000));
        
        for(int i = 0; i<3; i++)
        {
            byte[i]++;
        }
      
        recieveByte(handle);
        
       
       
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






