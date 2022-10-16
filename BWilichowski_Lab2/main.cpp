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

I2C tempsensor(I2C_SDA0, I2C_SCL0); // sda, scl
DigitalOut SetHigh(p32); //P1.0


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
    thread_sleep_for(1000);
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
    thread_sleep_for(1000);
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

   
    char subaddr[8];
    char data[0];

    
    //subaddr[1] = 0X00;

    uint8_t test;
    uint8_t test1;

    uint16_t MostSig;
    uint16_t LestSig;
    uint16_t Combo;

//who am i 
    subaddr[0] = 0XD0;
    tempsensor.write(writeaddr, (const char*) subaddr, 1, true); //setting SDA and SCL? 
    tempsensor.read(readaddr, data, 1, false); //setting SDA and SCL?
   //MyMessage.printf("Output from Who am I register: %i \r\n", test);
   // MyMessage.printf("Output from Who am I register: %i \r\n", test1);
    
//ac6
    subaddr[0] = 0xB4;
    tempsensor.write(writeaddr, (const char*) subaddr, 1, true); //setting SDA and SCL? 
    tempsensor.read(readaddr, data, 1, false); //setting SDA and SCL?
    MostSig = data[0];
    
    subaddr[0] = 0xB5;
    tempsensor.write(writeaddr, (const char*) subaddr, 1, true); //setting SDA and SCL? 
    tempsensor.read(readaddr, data, 1, false); //setting SDA and SCL?
    LestSig = data[0];
    uint16_t AC6 = ((MostSig<<8)|LestSig);
    MyMessage.printf("AC6 coefficient: %i \r\n", AC6);

//ac5
    subaddr[0] = 0xB2;
    tempsensor.write(writeaddr, (const char*) subaddr, 1, true); //setting SDA and SCL? 
    tempsensor.read(readaddr, data, 1, false); //setting SDA and SCL?
    MostSig = data[0];
    
    subaddr[0] = 0xB3;
    tempsensor.write(writeaddr, (const char*) subaddr, 1, true); //setting SDA and SCL? 
    tempsensor.read(readaddr, data, 1, false); //setting SDA and SCL?
    LestSig = data[0];
    uint16_t AC5 = ((MostSig<<8)|LestSig);
    MyMessage.printf("AC5 coefficient: %i \r\n", AC5);

//mc
    subaddr[0] = 0xBC;
    tempsensor.write(writeaddr, (const char*) subaddr, 1, true); //setting SDA and SCL? 
    tempsensor.read(readaddr, data, 1, false); //setting SDA and SCL?
    MostSig = data[0];
    
    subaddr[0] = 0xBD;
    tempsensor.write(writeaddr, (const char*) subaddr, 1, true); //setting SDA and SCL? 
    tempsensor.read(readaddr, data, 1, false); //setting SDA and SCL?
    LestSig = data[0];
    int16_t MC = ((MostSig<<8)|LestSig);
    MyMessage.printf("MC coefficient: %i \r\n", MC);
    
//md
    subaddr[0] = 0xBE;
    tempsensor.write(writeaddr, (const char*) subaddr, 1, true); //setting SDA and SCL? 
    tempsensor.read(readaddr, data, 1, false); //setting SDA and SCL?
    MostSig = data[0];

    subaddr[0] = 0xBF;
    tempsensor.write(writeaddr, (const char*) subaddr, 1, true); //setting SDA and SCL? 
    tempsensor.read(readaddr, data, 1, false); //setting SDA and SCL?
    LestSig = data[0];
    int16_t MD = ((MostSig<<8)|LestSig);
    MyMessage.printf("MD coefficient: %i \r\n", MD);


 /*  
//AC1     
    subaddr[0] = 0xAA;
    tempsensor.write(writeaddr, (const char*) subaddr, 1, true); //setting SDA and SCL? 
    tempsensor.read(readaddr, data, 1, false); //setting SDA and SCL?
    MostSig = data[0];
      
    subaddr[0] = 0xAB;
    tempsensor.write(writeaddr, (const char*) subaddr, 1, true); //setting SDA and SCL? 
    tempsensor.read(readaddr, data, 1, false); //setting SDA and SCL?
    LestSig = data[0];
    uint16_t AC1 = ((MostSig<<8)|LestSig);
    MyMessage.printf("AC1 coefficient: %i \r\n", AC1);
    
//AC2
    subaddr[0] = 0xAC;
    tempsensor.write(writeaddr, (const char*) subaddr, 1, true); //setting SDA and SCL? 
    tempsensor.read(readaddr, data, 1, false); //setting SDA and SCL?
    MostSig = data[0];
       
    subaddr[0] = 0xAD;
    tempsensor.write(writeaddr, (const char*) subaddr, 1, true); //setting SDA and SCL? 
    tempsensor.read(readaddr, data, 1, false); //setting SDA and SCL?
    LestSig = data[0];
    uint16_t AC2 = ((MostSig<<8)|LestSig);
    MyMessage.printf("AC2 coefficient: %i \r\n", AC2);
    
//AC3
    subaddr[0] = 0xAE;
    tempsensor.write(writeaddr, (const char*) subaddr, 1, true); //setting SDA and SCL? 
    tempsensor.read(readaddr, data, 1, false); //setting SDA and SCL?
    MostSig = data[0];

    subaddr[0] = 0xAF;
    tempsensor.write(writeaddr, (const char*) subaddr, 1, true); //setting SDA and SCL? 
    tempsensor.read(readaddr, data, 1, false); //setting SDA and SCL?
    LestSig = data[0];
    uint16_t AC3 = ((MostSig<<8)|LestSig);
    MyMessage.printf("AC3 coefficient: %i \r\n", AC3);
        
//AC4
    subaddr[0] = 0xB0;
    tempsensor.write(writeaddr, (const char*) subaddr, 1, true); //setting SDA and SCL? 
    tempsensor.read(readaddr, data, 1, false); //setting SDA and SCL?
    MostSig = data[0];  

    subaddr[0] = 0xB1;
    tempsensor.write(writeaddr, (const char*) subaddr, 1, true); //setting SDA and SCL? 
    tempsensor.read(readaddr, data, 1, false); //setting SDA and SCL?
    LestSig = data[0];
    uint16_t AC4 = ((MostSig<<8)|LestSig);
    MyMessage.printf("AC4 coefficient: %i \r\n", AC4);
            
//B1   
    subaddr[0] = 0xB6;
    tempsensor.write(writeaddr, (const char*) subaddr, 1, true); //setting SDA and SCL? 
    tempsensor.read(readaddr, data, 1, false); //setting SDA and SCL?
    MostSig = data[0];

    subaddr[0] = 0xB7;
    tempsensor.write(writeaddr, (const char*) subaddr, 1, true); //setting SDA and SCL? 
    tempsensor.read(readaddr, data, 1, false); //setting SDA and SCL?
    LestSig = data[0];
    uint16_t B1 = ((MostSig<<8)|LestSig);
    MyMessage.printf("B1 coefficient: %i \r\n", B1);
    
//B2
    subaddr[0] = 0xB8;
    tempsensor.write(writeaddr, (const char*) subaddr, 1, true); //setting SDA and SCL? 
    tempsensor.read(readaddr, data, 1, false); //setting SDA and SCL?
    MostSig = data[0];

    subaddr[0] = 0xB9;
    tempsensor.write(writeaddr, (const char*) subaddr, 1, true); //setting SDA and SCL? 
    tempsensor.read(readaddr, data, 1, false); //setting SDA and SCL?
    LestSig = data[0];
    uint16_t B2 = ((MostSig<<8)|LestSig);
    MyMessage.printf("B2 coefficient: %i \r\n", B2);

//MB
    subaddr[0] = 0xBa;
    tempsensor.write(writeaddr, (const char*) subaddr, 1, true); //setting SDA and SCL? 
    tempsensor.read(readaddr, data, 1, false); //setting SDA and SCL?
    MostSig = data[0];

    subaddr[0] = 0xBB;
    tempsensor.write(writeaddr, (const char*) subaddr, 1, true); //setting SDA and SCL? 
    tempsensor.read(readaddr, data, 1, false); //setting SDA and SCL?
    LestSig = data[0];
    uint16_t MB = ((MostSig<<8)|LestSig);
    MyMessage.printf("B2 coefficient: %i \r\n", MB);    
   
    */
    

    while(true)
    { 
    char temp[2];
    temp[0] = 0xF4;
    temp[1] = 0x2E;
    char hold[2];
    uint16_t MSB;
    uint16_t LSB;
    uint16_t UT;
   
    tempsensor.write(writeaddr, (const char*) temp, 2, true);
    thread_sleep_for(5);

    temp[0] = 0xF6;
    tempsensor.write(writeaddr, (const char*) temp, 1, true);
    tempsensor.read(readaddr, hold, 1, false);
    
    MSB = hold[0];
    LSB = hold[1];
    UT = ((MSB << 8)|LSB);
    MyMessage.printf("MSB: %i \n\r", hold[0]);
    MyMessage.printf("LSB: %i \n\r", hold[1]);
    MyMessage.printf("Uncompensated temperature value: %i \n\r", UT);

    int16_t X1 = (UT - AC6)* AC5 / pow(2,15);
    int16_t X2 = MC * pow(2,11) / (X1 + MD);
    int16_t B5 = X1 + X2;
    int16_t T = (B5 + 8) / pow(2,4);

    MyMessage.printf("True Temperature: %i\n\r", T);
    }

}

// main() runs in its own thread in the OS
int main()
{
    SetHigh = 1;
    tick.attach(&flipflop, 2);
    //thread1.start(read_temperature);
   // thread2.start(read_pressure);
    thread3.start(ItoC);
   
    

    while (true) 
    {
    thread_sleep_for(5000);
    }
}

