#include <iostream>
//#include <iomanip>
#include "main.hpp"
#include <iomanip>
#include <string.h> 



libusb_device* findProgrammer()
{
    
    libusb_device **list;
    ssize_t count = libusb_get_device_list(nullptr, &list);
    libusb_device* programmer = nullptr;

    for(int i = 0; i<count; i++)
    {
        libusb_device *device = list[i];
        
        // get descriptor
        libusb_device_descriptor descriptor;
        bailOnError(libusb_get_device_descriptor(device, &descriptor));
        // just try to get the VID and PID.
        // VID 04d8 (microchip technology)
        // PID 3939 (Arbitrary!)

        if(descriptor.idVendor == 0x4d8 && descriptor.idProduct == 0x00DD)
        {
            programmer = device;
            libusb_ref_device(programmer);
            break;
        }
    }
    
    // we have to discard the list right 
    libusb_free_device_list(list, true);
    return programmer;
}


void enumerateProgrammer(libusb_device* programmer)
{

    std::cout << "Yay, we found the programmer!!!!!!!!1!!!\n";
    // get the number of configuration descriptors:
    libusb_device_descriptor devDesc;
    bailOnError(libusb_get_device_descriptor(programmer, &devDesc));
   
    
  
    std::cout << (int)devDesc.bNumConfigurations << " Configurations.\n";
    std::cout << (int)devDesc.bDeviceClass << " Device class\n";
    std::cout << (int)devDesc.bDeviceSubClass << " Subclass\n";
    std::cout << (int)devDesc.bDeviceProtocol << " Protocol\n\n";

    libusb_config_descriptor* confDesc;
    bailOnError(libusb_get_config_descriptor(programmer, 0, &confDesc));
  

    std::cout << (int)confDesc->bNumInterfaces << " Num interfaces\n";

    // try to iterate through interfaces?
    for(int i = 0; i<confDesc->bNumInterfaces; i++)
    {
        libusb_interface_descriptor interface = confDesc->interface[i].altsetting[0];
        std::cout << "interface #"<<i<<":\n";

        std::cout<< "\tclass: "<<(int)interface.bInterfaceClass<<"\n";
        std::cout<< "\tsubclass: "<<(int)interface.bInterfaceSubClass<<"\n";
        std::cout<< "\tprotocol: "<<(int)interface.bInterfaceProtocol<<"\n";
        std::cout<< "\tnum endpoints: "<<(int)interface.bNumEndpoints<<"\n";
        // iterate through endpoints?
        for(int j = 0; j<interface.bNumEndpoints; j++)
        {
            std::cout<<"\tEndpoint #"<<j<<":\n";
            libusb_endpoint_descriptor endpoint = interface.endpoint[j];
            bool directionIn = endpoint.bEndpointAddress & 0x80;
            int addr = endpoint.bEndpointAddress & 0x0f;
            std::cout<<"\t\tAddress: "<< std::hex << addr << (directionIn?" In":" Out") << "\n";
            std::cout<<"\t\tAttributes: "<<std::hex << (int)endpoint.bmAttributes << "\n";
            std::cout<< "\t\tMax packet Size:"<<endpoint.wMaxPacketSize<<"\n";
        }
    }

}

libusb_device_handle* openProgrammer(libusb_device * programmer)
{
    libusb_device_handle* handle;
    bailOnError(libusb_open(programmer, &handle));
    std::cout<<"\nOpened device for communciations.\n";

    bailOnError(libusb_set_auto_detach_kernel_driver(handle, 1));
    // claim the cdc and hid interfaces.
    bailOnError(libusb_claim_interface(handle, CDC_INTERFACE));
    bailOnError(libusb_claim_interface(handle, CDC_DATA_INTERFACE));
    bailOnError(libusb_claim_interface(handle, HID_INTERFACE));
    std::cout<<"Claimed interfaces.\n";

    return handle;
}

void closeProgrammer(libusb_device* programmer, libusb_device_handle* handle)
{
    if(handle)
    {
        bailOnError(libusb_release_interface(handle, CDC_INTERFACE));
        bailOnError(libusb_release_interface(handle, CDC_DATA_INTERFACE));
        bailOnError(libusb_release_interface(handle, HID_INTERFACE));
        libusb_close(handle);
    }

    // dereference device
    libusb_unref_device(programmer);
}