#include "Comms.hpp"
#include <iostream>

uint8_t doCommsCycle(libusb_device_handle* programmer, uint16_t addr, RW rw, uint8_t data)
{
    // first combine the data into what we actually want.
    uint8_t byte2 = (addr>>8)|(rw<<7);
    uint8_t toSend[] = { data, byte2, (uint8_t)(addr&0x00FF) };

    // yell about it (for now)
    //std::cout<<"Sent 0x"<<std::hex<<(int)toSend[0]<<", 0x"<<(int)toSend[1]<<", 0x"<<(int)toSend[2]<<".\n";

       
    // Do the transfer. Pretty simple!
    bailOnError(libusb_bulk_transfer(programmer, UART_ENDPOINT|USB_OUT, &toSend[0], 3, nullptr, TIMEOUT));  
    return receiveByte(programmer);
    
}


uint8_t readByte(libusb_device_handle* programmer, uint16_t addr)
{
    return doCommsCycle(programmer, addr, RW::R, 0x00);
}

void writeByte(libusb_device_handle *programmer, uint16_t addr, uint8_t data)
{
    doCommsCycle(programmer, addr, RW::W, data);
}

uint8_t receiveByte(libusb_device_handle* programmer)
{
    uint8_t toReturn;
    int transfered = 0;
    int tries = 0;
    
    // the purpose of this loop is to make sure that we actually get a byte. 
    // Probably because of the half clock cycle delay between the stop bit of the last sent byte and the start bit of 
    // the received byte, sometimes the first time we try to get a byte it won't be there yet. We'll try a few times to 
    // make sure we get it.
    while(transfered==0)
    {
        bailOnError(libusb_bulk_transfer(programmer, UART_ENDPOINT|USB_IN, &toReturn, 1, &transfered, TIMEOUT));
        tries++;
        if(tries>=3)
        {
            std::cout<<"ERROR: receiveByte failed to get data.\n";
            std::exit(1);
        }
    
    }
    
    return toReturn;

}

bool maybeReceiveByte(libusb_device_handle* programmer, uint8_t* byte)
{
    // now recive a byte

    int transfered = 0;
    int tries = 0;
    
    while(transfered==0)
    {
        tries++;
        // we don't care if this times out. Other errors would be an issue, but... it's probably fine. (totally).
        libusb_bulk_transfer(programmer, UART_ENDPOINT|USB_IN, byte, 1, &transfered, 50);
        
        if(tries>=3)
        {
            return false;
        }
    
    }

    return true;
   
}


// ensures the programmer is in an acceptable state to read/write an eeprom. (state machine is 0, OS(?) USB data buffer cleared)
void confirmInitialState(libusb_device_handle* programmer)
{
    // First, clear the output (from the programmer) buffer.
    // consume any buffered input so we start off on the right foot.
    int transfered =1;
    uint8_t received;
    while(!libusb_bulk_transfer(programmer, UART_ENDPOINT|USB_IN, &received, 1, &transfered, 50)){}

    // While there is a power on reset circuit, we can't neccesarily trust it. Even if it works, the shift registers will have random data in them...
    // this could be fixed in hardware... hold that thought.

    // if not here's code
    uint32_t data = 0xFFFFFFFF;
    
    // garuntee the device is at the initial state by waiting until it sends data back.
    while(!maybeReceiveByte(programmer, &received))
    {
        
        bailOnError(libusb_bulk_transfer(programmer, UART_ENDPOINT|USB_OUT, (uint8_t*)&data, 1, nullptr, TIMEOUT));
    }
    // do it again. Since we can't garuntee that the device was at the inital state
    // but the shift registers might still contain garbage if the device wasn't.
    // running it again will clear everything out.

    while(!maybeReceiveByte(programmer, &received))
    {
      
        bailOnError(libusb_bulk_transfer(programmer, UART_ENDPOINT|USB_OUT, (uint8_t*)&data, 1, nullptr, TIMEOUT));
    }
    
    std::cout<<"Initialized. It is now safe to insert the EEPROM.\n";
    std::cin.get();

}