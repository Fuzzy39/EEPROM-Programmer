#include <iostream>
#include "main.hpp"
#include <libusb-1.0/libusb.h>

int main(void)
{
    int err;

    libusb_init_option options[] =  {{LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_INFO}};

    if(libusb_init_context(nullptr, options,1))
    {
        std::cout<<"Failed to Initialize.\n";
        return 1;
    }
    
    // try to enumerate devices
    libusb_device **list;

    ssize_t count = libusb_get_device_list(nullptr, &list);
    std::cout<<"Found "<< count <<" devices.\n";

    for(int i = 0; i<count; i++)
    {
        libusb_device *device = list[i];
        // the goal here is to get the name of the product string..
        //std::cout<<"Getting desc.\n";
        libusb_device_descriptor descriptor;
        if(err = libusb_get_device_descriptor(device, &descriptor))
        {
            std::cout<<"ERROR: "<<libusb_error_name(err)<<"\n";
            return err;
        }
        
        // we have to open the device to get a string from it.
        libusb_device_handle* handle;
        if(err = libusb_open(device, &handle))
        {
            std::cout<<"ERROR: "<<"Unknown device..."<<"\n";
            continue;
        }

        //std::cout<<"Getting name...\n";
        unsigned char productStr[1000] ;
        if(err = libusb_get_string_descriptor_ascii(handle, descriptor.idProduct, productStr, 1000))
        {
            std::cout<<"ERROR: "<<libusb_error_name(err)<<"\n";
            //return err;
            //continue;
        }

        libusb_close(handle);

        std::cout<<productStr<<"\n";
    }
    


    // we have to discard the list right 
    libusb_free_device_list(list, true);

    libusb_exit(nullptr);
    std::cout<<"Sucess, maybe!\n";
    return 0;
}