#include <iostream>
//#include <iomanip>
#include "main.hpp"
#include <iomanip>
#include <string.h> 
#include <cmath>



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

void setSpeed(libusb_device_handle* handle, DeviceSpeed speed)
{
    int dividerSlow = 3;
    // baud, fortunately, isn't too bad to calculate.
    const int MAX_CLOCK = 12000000; // 12 mhz
    int baud = MAX_CLOCK>>speed>>dividerSlow; // the 2 is there because of the current placement of the jumper wire.
    std::cout<<"Setting speed to "<<std::dec<<baud<<" baud\n";

    unsigned char data[] = {0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x08}; // according to page 32 in the usb pstn doc, this will set line coding.
    memcpy(data, &baud, sizeof(baud)); // put the baud rate in the buffer (this is really C code, isn't it)


    // SET_LINE_CODING 
    int transfered = libusb_control_transfer(handle, 0b00100001, 0x20, 0, CDC_ENDPOINT, data, sizeof(data), TIMEOUT);
    if(transfered!= sizeof(data))
    {
        std::cout<<"Didn't transfer all of the data I guess? transfer returned:"<<transfered<<"\n";
    }


    // now we update the clock settings in the hid device.
    // okay, whatever, logs!
    int baudtoMCP = MAX_CLOCK>>(speed-5); // compensate for hardware divider
    std::cout<<"MCP speed: "<<baudtoMCP<<"\n";
    int shiftsToMCP = (log(12000000/baudtoMCP)/log(2))+2; //the counter divides by 32, the max clock speed 
    std::cout<<"shifts to MCP:"<<shiftsToMCP<<"\n";

    // we have to send a command to the HID device now...
    size_t commandSize = 0x40;
    u_int8_t * command = (u_int8_t*)calloc(commandSize, sizeof(u_int8_t));
    command[0]=0x60; // set SRAM settings
    // change the clock speed
    command[2]=0b10010000 | shiftsToMCP;
    // we can leave everything else 0 and we should be fine

    bailOnError(libusb_interrupt_transfer(handle, HID_ENDPOINT|OUT, command, commandSize, &transfered, TIMEOUT));
    if(transfered!=commandSize)
    {
        std::cout<<"Could not set clock speed\n";
        std::exit(100);
    }
    // get the response back
    bailOnError(libusb_interrupt_transfer(handle, HID_ENDPOINT|IN, command, commandSize, &transfered, TIMEOUT));
    if(command[1] || transfered != commandSize )
    {
        std::cout<<"There was an error setting clock speed.\n";
        std::exit(101);
    }

  

    free(command);


    
}