#include "mbed.h"
#include "BWilichowski_binaryutils.h"
#include "Ticker.h"
#include "USBSerial.h"
#include "I2C.h"
#include "DigitalOut.h"

#define temperature (1UL << 0) //temperature flag
#define pressure (1UL << 1) //pressure flag

#define LEDDIR (uint32_t*) 0x50000514  //In is ON, OUT is OFF
#define LEDSET (uint32_t*) 0x50000508 //Set LED Pin 
#define LEDCLEAR (uint32_t*) 0x5000050c //Clear LED Pin



const int writeaddr = 0xEE; //write for board
const int readaddr = 0xEF; //read for board

Ticker tick;
EventFlags event_flags;
Thread thread1;
Thread thread2;
Thread thread3;
USBSerial MyMessage;

I2C tempsensor(p14, p15); // sda, scl
DigitalOut I2C_SCL(p14);
DigitalOut I2C_SDA(p15);

bool state;

void flipflop()
{
    if(state == 0)
    {
        event_flags.set(temperature); //set temp
        event_flags.clear(pressure); //clear pressure
        state = 1;  
    }

    else if(state == 1)
    {
       event_flags.set(pressure); //set pressure
       event_flags.clear(temperature); //clear temp
       state = 0;
    }
    //when ticker counts to xx, set flag
    //otherwise, set other flag

}

void read_temperature()
{
    setbit(LEDDIR, 6);
    while(true)
    {
    event_flags.wait_all(temperature);
    MyMessage.printf("Pressure LED OFF, Temp LED ON \n\r");
    setbit(LEDCLEAR, 6);
    //MyMessage.printf("Temperature Led ON \n\r");
    setbit(LEDSET, 6);
    //MyMessage.printf("Temperature Led OFF \n\r");
    //MyMessage.printf("Pressure LED ON \n\r");

    }
}

void read_pressure()
{
    setbit(LEDDIR, 24);
    while(true)
    {
    event_flags.wait_all(pressure);
    MyMessage.printf("Pressure LED ON, Temp LED OFF \n\r");
    setbit(LEDCLEAR, 24);
    //MyMessage.printf("Pressure LED ON \n\r");
    setbit(LEDSET, 24);
    //MyMessage.printf("Pressure LED OFF \n\r");
    //MyMessage.printf("Temperature Led ON \n\r");
    }
}

void ItoC()
{

    //Write(addr(chip address (0xEE)), subaddress(0xD0h), 1 byte long, false/true we want true)
    //read(addr(chip address (0xEF)), data (memory address), 1, false) //returns a uint8 = char
    //A1 = data[0]
    //combined = 0000 0000 0000 0000 OR w/ data AB, shift data AA, OR together
    //data_aa data_ab --- (data_aa << 8) need to OR these two together to get 16 bit value

    //DigitalOut(p1.1);
    const char* subaddr[8]; 
    subaddr[0] = 0xD0;
    const char* data[8];

    tempsensor.write(writeaddr, subaddr[0], 1, true); //setting SDA and SCL?
    data[0] = subaddr[0];
    tempsensor.read(readaddr, data[0], 1, false); //setting SDA and SCL?

    while(true)
    {

    }

}

// main() runs in its own thread in the OS
int main()
{
    tick.attach(&flipflop, 2);
    thread1.start(read_temperature);
    thread2.start(read_pressure);
    thread3.start(ItoC);

    while (true) 
    {
    thread_sleep_for(5000);
    }
}

