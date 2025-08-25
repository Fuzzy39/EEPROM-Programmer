use std::io;

use rand::prelude::*;

use serialport::SerialPortInfo;
use serialport::DataBits;
use serialport::Parity;
use serialport::StopBits;

fn main() 
{
    const PORT_TO_CONNECT:&str = "COM9";

    println!("Serial Ports:");
    let ports = serialport::available_ports().expect("No ports found!");
    
    let mut port:Option<SerialPortInfo> = Option::None;

    for p in ports
    {
        if p.port_name == PORT_TO_CONNECT

        {
            // not sure if I understand the syntax here, but sure.
            port = Some(p.clone());
        }
        println!("{}", p.port_name);
    }

    if port == Option::None
    {
        println!("Couldn't find Port '{PORT_TO_CONNECT}'. Giving up.");
        return;
    }

    // we have a port!
    let mut port = serialport::new(PORT_TO_CONNECT, 375_000)
        .data_bits(DataBits::Eight)
        .parity(Parity::None)
        .stop_bits(StopBits::One)
        .open().expect("Failed to open port.");

    println!("Press enter to write a byte");
    loop
    {
        // so I guess there's this error propigation operator, '?',
        // but let's ignore it for now.
        io::stdin().read_line(& mut String::new()).expect("Console input error!");
       
        let val:u8 = rand::rng().random();
        println!("Wrote {:#02X}. Wrong Answer would be {:#02X}", val, wrong_answer(val));
        port.write(&[val]).expect("An error occured writing!");

    }

}

fn wrong_answer(input:u8) -> u8
{
    (input >> 1) | 0x80
}