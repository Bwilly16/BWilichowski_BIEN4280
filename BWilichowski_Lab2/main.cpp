#include "mbed.h"
#include "BWilichowski_binaryutils.h"
#include "Ticker.h"
#include "USBSerial.h"

#define temperature (1UL << 0) //temperature flag
#define pressure (1UL << 1) //pressure flag

#define LEDDIR (uint32_t*) 0x50000514  //In is ON, OUT is OFF
#define LEDSET (uint32_t*) 0x50000508 //Set LED Pin 
#define LEDCLEAR (uint32_t*) 0x5000050c //Clear LED Pin



Ticker tick;
EventFlags event_flags;
Thread thread1;
Thread thread2;
USBSerial MyMessage;

//I2C i2c(I2C_SDA, I2C_SCL); 

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

// main() runs in its own thread in the OS
int main()
{
    tick.attach(&flipflop, 2);
    thread1.start(read_temperature);
    thread2.start(read_pressure);

    while (true) 
    {
    thread_sleep_for(5000);
    }
}

