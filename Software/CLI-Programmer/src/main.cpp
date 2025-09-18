#include <iostream>
//#include <iomanip>
#include "main.hpp"
#include <iomanip>
#include <string.h> 


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

    // okay, I guess things might be being handled by kernel drivers. No good!

    // we have to claim interfaces!



    // now we try to communicate.... how?
    // well, first, open the device.
    libusb_device_handle* handle;
    bailOnError(libusb_open(programmer, &handle));
    std::cout<<"\nOpened device...\n";

    bailOnError(libusb_set_auto_detach_kernel_driver(handle, 1));
    // claim the cdc interfaces
    bailOnError(libusb_claim_interface(handle, 0));
    bailOnError(libusb_claim_interface(handle, 1));

    std::cout<<"claimed interfaces!\n";
    
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
    while(true)
    {
       
        std::cout<<"Attempting to send data:"<<(int)byte<<"\n";

        // try to send a number?
      
        bailOnError(libusb_bulk_transfer(handle, 0x02, &byte, 1, nullptr, 3000));
        // fingers crossed!

        byte++;
       
        std::cin.get();
    }


    bailOnError(libusb_release_interface(handle, 0));
    bailOnError(libusb_release_interface(handle, 1));
    libusb_close(handle);



    // dereference device
    libusb_unref_device(programmer);
    libusb_exit(nullptr);
    return 0;
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

        if(descriptor.idVendor == 0x4d8 && descriptor.idProduct == 0x3939)
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

void bailOnError(int err)
{
    if(!err) return;
    std::cout<<"ERROR: "<<libusb_error_name(err)<<"\n";
    std::exit(err);
}
