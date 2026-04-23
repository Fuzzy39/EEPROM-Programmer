#pragma once
#include <libusb-1.0/libusb.h>
#include <cstdint>
#include <string> 
#include "main.hpp"
#include <chrono>

enum RW
{
    R = 0x01,       // M>>5 375khz (right now: 46.875khz)
    W = 0x00,        // M>>10 11.719khz (1465 hz right now)
};


bool readRomAndVerify(libusb_device_handle* handle, std::string verifyFromPath, size_t size);
bool verifyRom(libusb_device_handle* handle, uint8_t* actual, size_t size);
void readRom(libusb_device_handle* handle, std::string writeTo, size_t size);

// page size should probably be a power of 2.
bool writeRom(libusb_device_handle* handle, size_t size, uint8_t* data, size_t pageSize, std::chrono::microseconds delayBetweenPageWrites);



// Basic coms for the device.
uint8_t doCommsCycle(libusb_device_handle* programmer, uint16_t addr, RW rw, uint8_t data);
uint8_t readByte(libusb_device_handle* programmer, uint16_t addr);
void writeByte(libusb_device_handle* programmer, uint16_t addr, uint8_t data);



uint8_t receiveByte(libusb_device_handle* programmer);
bool maybeReceiveByte(libusb_device_handle* programmer, uint8_t* byte);


// ensures the programmer is in an acceptable state to read/write an eeprom. (state machine is 0, OS(?) USB data buffer cleared)
void confirmInitialState(libusb_device_handle* programmer);
