#include <iostream>
//#include <iomanip>
#include "main.hpp"
#include <iomanip>
#include <string.h> 


void listDevices()
{
    
     // try to enumerate devices
    libusb_device **list;

    ssize_t count = libusb_get_device_list(nullptr, &list);
    std::cout<<"Found "<< count <<" devices.\nVID | PID\n";

    for(int i = 0; i<count; i++)
    {
        libusb_device *device = list[i];
   
        // get descriptor
        libusb_device_descriptor descriptor;
        bailOnError(libusb_get_device_descriptor(device, &descriptor));
    
        // print the PID and VID (in the reverse order)
        std::cout << std::setfill ('0') << std::setw(sizeof(uint16_t)*2) 
       << std::hex << descriptor.idVendor << "|";
       
       std::cout <<std::setfill ('0') << std::setw(sizeof(uint16_t)*2) 
       << std::hex << descriptor.idProduct << "\n";

    }

    // we have to discard the list right 
    libusb_free_device_list(list, true);

}


void bailOnError(int err)
{
    if(!err) return;
    std::cout<<"ERROR: "<<libusb_error_name(err)<<"\n";
    std::exit(err);
}
