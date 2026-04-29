#include "Comms.hpp"
#include <iostream>
#include <iomanip>
#include <thread>

bool readRomAndVerify(libusb_device_handle* handle, std::string verifyFromPath, size_t size)
{
    setSpeed(handle, DeviceSpeed::HIGH); // comms is unreliable on low speed, for some reason.
    confirmInitialState(handle);

    // read rom file!
    uint8_t* actual = (uint8_t*)malloc(size);
    FILE* file = fopen(verifyFromPath.c_str(), "rb");
    fread(actual, 1, size, file);
    fclose(file);


    bool toReturn = verifyRom(handle, actual, size);
    free(actual);
    return toReturn;
}

bool verifyRom(libusb_device_handle* handle, uint8_t* actual, size_t size)
{

    uint8_t* buffer = (uint8_t*)malloc(size);
    bool success = true;

    for(uint16_t i= 0;  i<size; i++)
    {
        if(i%(1024/4)==0)
        {
            std::cout<<"0x"<<std::hex<<std::setw(4) << std::setfill('0')<<(int)i<<"...\n";
        }
        uint8_t data = readByte(handle, i); // i
        //std::cout<<"Got: 0x"<<std::hex<<(int)data<<"\n";
        buffer[i] = data;
        uint8_t expected = actual[i];

        if(data != expected)
        {
            std::cout<<"Error at 0x"<<std::hex<<std::setw(4) << std::setfill('0')<<(int)i<<": Got 0x"
                <<std::setw(2)<<(int)buffer[i]<<", Expected 0x"<<(int)expected<<".";
            std::cin.get();
            success = false;

        }

    }

    free(buffer);
    std::cout<<(success? "Done! Contents verified.\n": "Done. Failed to Verify.\n");
    return success;
}

void readRom(libusb_device_handle* handle, std::string writeTo, size_t size)
{
    setSpeed(handle, DeviceSpeed::HIGH); // comms is unreliable on low speed, for some reason.
    confirmInitialState(handle);


    // read data off the rom.
    uint8_t* buffer = (uint8_t*)malloc(size);
    for(uint16_t i= 0;  i<size; i++)
    {
        if(i%(1024/4)==0)
        {
            std::cout<<"0x"<<std::hex<<std::setw(4) << std::setfill('0')<<(int)i<<"...\n";
        }
        uint8_t data = readByte(handle, i); // i
        buffer[i] = data;

    }

    // write to output.
    FILE* file = fopen(writeTo.c_str(), "wb");
    fwrite(buffer, 1, size, file);
    fclose(file);
    std::cout<<"Done! Wrote to '"<<writeTo.c_str()<<"'.\n";

}


bool writeRom(libusb_device_handle* handle, size_t size, std::string writeFrom, size_t pageSize, std::chrono::microseconds delayBetweenPageWrites)
{
    setSpeed(handle, DeviceSpeed::HIGH); // comms is unreliable on low speed, for some reason.
    confirmInitialState(handle);

    // read data file
    uint8_t* data = (uint8_t*)malloc(size);
    FILE* file = fopen(writeFrom.c_str(), "rb");
    fread(data, 1, size, file);
    fclose(file);

    for(int i = 0; i<size/pageSize; i++)
    {
        for(int j = 0; j<pageSize; j++)
        {
            size_t index = i*pageSize+j;
            if(index%(1024/4)==0)
            {
                std::cout<<"Writing: 0x"<<std::hex<<std::setw(4) << std::setfill('0')<<(int)(i*pageSize)<<"...\n";
            }
            uint8_t toWrite = data[index];
            writeByte(handle, index, toWrite, false);
        }
        // delay 
        // we could speed this up probably by reducing the delay by the time it'll take to send all the data over, but it's not very much.
        // ~64 us compared to 10ms.
        std::this_thread::sleep_for(delayBetweenPageWrites);
    }   

    // clear the input buffer if it's built up (garbage often does)
    /*uint8_t* garbage = (uint8_t*)malloc(size);
    int transfered;
    libusb_bulk_transfer(handle, UART_ENDPOINT|USB_IN, garbage, size, &transfered, 50);
    free(garbage);*/
   
    // verify rom.
    std::cout<<"Data written. Verifying...\n\n";
    bool toReturn = verifyRom(handle, data, size);
    free(data);
    return toReturn;
}




uint8_t doCommsCycle(libusb_device_handle* programmer, uint16_t addr, RW rw, uint8_t data, bool lazy)
{
    // first combine the data into what we actually want.
    uint8_t byte2 = (addr>>8)|(rw<<7);
    uint8_t toSend[] = { data, byte2, (uint8_t)(addr&0x00FF) };

    // yell about it (for now)
    //std::cout<<"Sent 0x"<<std::hex<<(int)toSend[0]<<", 0x"<<(int)toSend[1]<<", 0x"<<(int)toSend[2]<<".\n";

       
    // Do the transfer. Pretty simple!
    bailOnError(libusb_bulk_transfer(programmer, UART_ENDPOINT|USB_OUT, &toSend[0], 3, nullptr, TIMEOUT));  
    if(!lazy) return receiveByte(programmer);

    // if we don't care about the return value, we basically toss it.
    // there's some kind of read buffer somewhere, and it's going to fill with garbage.
    // but we're being lazy. this will need to be dealt with before actually reading, though.
    return 0;
    
}


uint8_t readByte(libusb_device_handle* programmer, uint16_t addr)
{
    return doCommsCycle(programmer, addr, RW::R, 0x00, false);
}

void writeByte(libusb_device_handle *programmer, uint16_t addr, uint8_t data, bool lazy)
{
    doCommsCycle(programmer, addr, RW::W, data, lazy);
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
        //if(tries == 2) std::cout<<"Honk!";
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