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



Ticker tick;
EventFlags event_flags;
Thread thread1;
Thread thread2;
Thread thread3;
USBSerial MyMessage;

I2C tempsensor(I2C_SDA0, I2C_SCL0); // sda, scl
DigitalOut SetHigh(p32); //P1.0
Mutex locker_mutex;



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
//locker_mutex.lock();
const int writeaddr = 0xEE; //write for board
const int readaddr = 0xEF; //read for board
//Coefficients
uint16_t AC5 = 0;
uint16_t AC6 = 0;
int16_t MD = 0;
int16_t MC = 0;


int32_t X1 = 0;
int32_t X2 = 0;
int32_t UT = 0;
int32_t B5 = 0;
int32_t T = 0;


char temp[2];
char hold[2];

uint16_t MSB = 0;
uint16_t LSB = 0;

uint16_t MostSig = 0;
uint16_t LestSig = 0;
char subaddr[2];
char data[1];

//who am i 
    subaddr[0] = 0XD0;    
    tempsensor.write(writeaddr, (const char*) subaddr, 1, true); //setting SDA and SCL? 
    tempsensor.read(readaddr, data, 1, false); //setting SDA and SCL?
   //MyMessage.printf("Output from Who am I register: %i \r\n", data[0]);
  


//Temperature   
//ac6
    
    subaddr[0] = 0xB4;
    tempsensor.write(writeaddr, (const char*) subaddr, 1, true); //setting SDA and SCL? 
    tempsensor.read(readaddr, data, 1, false); //setting SDA and SCL?
    MostSig = data[0];
    
    subaddr[0] = 0xB5;
    tempsensor.write(writeaddr, (const char*) subaddr, 1, true); //setting SDA and SCL? 
    tempsensor.read(readaddr, data, 1, false); //setting SDA and SCL?
    LestSig = data[0];
    AC6 = ((MostSig<<8)|LestSig);
    //MyMessage.printf("AC6 coefficient: %i \r\n", AC6);

//ac5

    subaddr[0] = 0xB2;
    tempsensor.write(writeaddr, (const char*) subaddr, 1, true); //setting SDA and SCL? 
    tempsensor.read(readaddr, data, 1, false); //setting SDA and SCL?
    MostSig = data[0];
    
    subaddr[0] = 0xB3;
    tempsensor.write(writeaddr, (const char*) subaddr, 1, true); //setting SDA and SCL? 
    tempsensor.read(readaddr, data, 1, false); //setting SDA and SCL?
    LestSig = data[0];
    AC5 = ((MostSig<<8)|LestSig);
   // MyMessage.printf("AC5 coefficient: %i \r\n", AC5);

//mc

    subaddr[0] = 0xBC;
    tempsensor.write(writeaddr, (const char*) subaddr, 1, true); //setting SDA and SCL? 
    tempsensor.read(readaddr, data, 1, false); //setting SDA and SCL?
    MostSig = data[0];
    
    subaddr[0] = 0xBD;
    tempsensor.write(writeaddr, (const char*) subaddr, 1, true); //setting SDA and SCL? 
    tempsensor.read(readaddr, data, 1, false); //setting SDA and SCL?
    LestSig = data[0];
    MC = ((MostSig<<8)|LestSig);
    //MyMessage.printf("MC coefficient: %i \r\n", MC);
    
//md

    subaddr[0] = 0xBE;
    tempsensor.write(writeaddr, (const char*) subaddr, 1, true); //setting SDA and SCL? 
    tempsensor.read(readaddr, data, 1, false); //setting SDA and SCL?
    MostSig = data[0];

    subaddr[0] = 0xBF;
    tempsensor.write(writeaddr, (const char*) subaddr, 1, true); //setting SDA and SCL? 
    tempsensor.read(readaddr, data, 1, false); //setting SDA and SCL?
    LestSig = data[0];
    MD = ((MostSig<<8)|LestSig);
   // MyMessage.printf("MD coefficient: %i \r\n", MD);


    setbit(LEDDIR, 6);
    
    while(true)
    {
    event_flags.wait_all(temperature);
   
    MyMessage.printf("Pressure LED OFF, Temp LED ON \n\r");
   
    setbit(LEDCLEAR, 6);
    setbit(LEDSET, 6);
     
    

    temp[0] = 0xF4;//address I want to write to 
    temp[1] = 0x2E;//what I want to write to the address
    tempsensor.write(writeaddr, (const char*) temp, 2, true); //writing 2E to the address of F4
    thread_sleep_for(5);

    temp[0] = 0xF6;
    tempsensor.write(writeaddr, (const char*) temp, 1, true);
    tempsensor.read(readaddr, hold, 1, false);
    MSB = hold[0];
   
    temp[0] = 0xF7;
    tempsensor.write(writeaddr, (const char*) temp, 1, true);
    tempsensor.read(readaddr, hold, 1, false);
    LSB = hold[0];
    UT = ((MSB<<8) + LSB);
    
    //actual calculations
    X1 = ((UT - AC6) * AC5/pow(2,15));
    X2 = ((MC * pow(2,11)) / (X1 + MD));
    B5 = (X1 + X2);
    T = ((B5 + 8) / pow(2,4));
    //MyMessage.printf("Past calculation section\n\r");

    MyMessage.printf("True Temperature in celsius: %d\n\r", T);//print out of live, true temperature 
    

    }
 // locker_mutex.unlock();  
}



void read_pressure()
{
//locker_mutex.lock();
const int writeaddr = 0xEE; //write for board
const int readaddr = 0xEF; //read for board

int32_t X1 = 0;
int32_t X2 = 0;
int32_t B5 = 0;
int16_t oss = 0;
int32_t p = 0;
int32_t UP = 0;
int32_t X3 = 0;
int32_t B3 = 0;        
uint32_t B4 = 0;
int32_t B6 = 0; 
uint32_t B7 = 0;

int16_t AC1 = 0;
int16_t AC2 = 0;
int16_t AC3 = 0;
uint16_t AC4 = 0;
int16_t MB = 0;
int16_t B2 = 0;
int16_t B1 = 0;



uint16_t MSB = 0;
uint16_t LSB = 0;
uint16_t XLSB = 0;

uint16_t MostSig = 0;
uint16_t LestSig = 0;

char subaddr[2];
char data[1];
char temp[2];
char hold[2];
char temp1[3];

//Pressure  
//AC1    
    subaddr[0] = 0xAA;
    tempsensor.write(writeaddr, (const char*) subaddr, 1, true); //setting SDA and SCL? 
    tempsensor.read(readaddr, data, 1, false); //setting SDA and SCL?
    MostSig = data[0];
      
    subaddr[0] = 0xAB;
    tempsensor.write(writeaddr, (const char*) subaddr, 1, true); //setting SDA and SCL? 
    tempsensor.read(readaddr, data, 1, false); //setting SDA and SCL?
    LestSig = data[0];
    AC1 = ((MostSig<<8)|LestSig);
   // MyMessage.printf("AC1 coefficient: %i \r\n", AC1);
    
//AC2
    subaddr[0] = 0xAC;
    tempsensor.write(writeaddr, (const char*) subaddr, 1, true); //setting SDA and SCL? 
    tempsensor.read(readaddr, data, 1, false); //setting SDA and SCL?
    MostSig = data[0];
       
    subaddr[0] = 0xAD;
    tempsensor.write(writeaddr, (const char*) subaddr, 1, true); //setting SDA and SCL? 
    tempsensor.read(readaddr, data, 1, false); //setting SDA and SCL?
    LestSig = data[0];
    AC2 = ((MostSig<<8)|LestSig);
   // MyMessage.printf("AC2 coefficient: %i \r\n", AC2);
    
//AC3
    subaddr[0] = 0xAE;
    tempsensor.write(writeaddr, (const char*) subaddr, 1, true); //setting SDA and SCL? 
    tempsensor.read(readaddr, data, 1, false); //setting SDA and SCL?
    MostSig = data[0];

    subaddr[0] = 0xAF;
    tempsensor.write(writeaddr, (const char*) subaddr, 1, true); //setting SDA and SCL? 
    tempsensor.read(readaddr, data, 1, false); //setting SDA and SCL?
    LestSig = data[0];
    AC3 = ((MostSig<<8)|LestSig);
   // MyMessage.printf("AC3 coefficient: %i \r\n", AC3);
        
//AC4
    subaddr[0] = 0xB0;
    tempsensor.write(writeaddr, (const char*) subaddr, 1, true); //setting SDA and SCL? 
    tempsensor.read(readaddr, data, 1, false); //setting SDA and SCL?
    MostSig = data[0];  

    subaddr[0] = 0xB1;
    tempsensor.write(writeaddr, (const char*) subaddr, 1, true); //setting SDA and SCL? 
    tempsensor.read(readaddr, data, 1, false); //setting SDA and SCL?
    LestSig = data[0];
    AC4 = ((MostSig<<8)|LestSig);
   // MyMessage.printf("AC4 coefficient: %i \r\n", AC4);
            
//B1   
    subaddr[0] = 0xB6;
    tempsensor.write(writeaddr, (const char*) subaddr, 1, true); //setting SDA and SCL? 
    tempsensor.read(readaddr, data, 1, false); //setting SDA and SCL?
    MostSig = data[0];

    subaddr[0] = 0xB7;
    tempsensor.write(writeaddr, (const char*) subaddr, 1, true); //setting SDA and SCL? 
    tempsensor.read(readaddr, data, 1, false); //setting SDA and SCL?
    LestSig = data[0];
    B1 = ((MostSig<<8)|LestSig);
   // MyMessage.printf("B1 coefficient: %i \r\n", B1);
    
//B2
    subaddr[0] = 0xB8;
    tempsensor.write(writeaddr, (const char*) subaddr, 1, true); //setting SDA and SCL? 
    tempsensor.read(readaddr, data, 1, false); //setting SDA and SCL?
    MostSig = data[0];

    subaddr[0] = 0xB9;
    tempsensor.write(writeaddr, (const char*) subaddr, 1, true); //setting SDA and SCL? 
    tempsensor.read(readaddr, data, 1, false); //setting SDA and SCL?
    LestSig = data[0];
    B2 = ((MostSig<<8)|LestSig);
   // MyMessage.printf("B2 coefficient: %i \r\n", B2);

//MB
    subaddr[0] = 0xBa;
    tempsensor.write(writeaddr, (const char*) subaddr, 1, true); //setting SDA and SCL? 
    tempsensor.read(readaddr, data, 1, false); //setting SDA and SCL?
    MostSig = data[0];

    subaddr[0] = 0xBB;
    tempsensor.write(writeaddr, (const char*) subaddr, 1, true); //setting SDA and SCL? 
    tempsensor.read(readaddr, data, 1, false); //setting SDA and SCL?
    LestSig = data[0];
    MB = ((MostSig<<8)|LestSig);


    setbit(LEDDIR, 24);
    
    while(true)
    {
   
    event_flags.wait_all(pressure);
    MyMessage.printf("Pressure LED ON, Temp LED OFF \n\r");
    setbit(LEDCLEAR, 24);
    setbit(LEDSET, 24);


//calculate pressure 

    temp1[0] = 0xF4; //address to write to
    temp1[1] = (0x34 + (oss<<6)); //what to write to address
   tempsensor.write(writeaddr, (const char*) temp1, 2, true);
   thread_sleep_for(10);

    temp1[0] = 0xF6;
    tempsensor.write(writeaddr, (const char*) temp1, 1, true);
    tempsensor.read(readaddr, hold, 1, false);
    MSB = hold[0];
   // MyMessage.printf("Most significant bit %i\n\r", MSB);

    temp1[0] = 0xF7;
    tempsensor.write(writeaddr, (const char*) temp1, 1, true);
    tempsensor.read(readaddr, hold, 1, false);
    LSB = hold[0];
    //MyMessage.printf("Least significant bit %i\n\r", LSB);

    temp1[0] = 0xF8;
    tempsensor.write(writeaddr, (const char*) temp1, 1, true);
    tempsensor.read(readaddr, hold, 1, false);
    XLSB = hold[0];
    UP = (((MSB<<16) + (LSB<<8) + XLSB)>>(8-oss));

 //Calculations   
    B6 = (B5 - 4000);
    X1 = (B2 * (B6 * B6 / pow(2,12)) / pow(2,11));
    X2 = ((AC2 * B6) / pow(2,11));
    X3 = (X1 + X2);
    B3 = (((AC1*4 + X3)<<oss)+2)/4;
    X1 = ((AC3*B6) / pow(2,13));
    X2 = (B1 * (B6 * B6 / pow(2,12)) / pow(2,16));
    X3 = (((X1 + X2) + 2) / pow(2,2));
    B4 = ((AC4 * (unsigned long)(X3 + 32768))/ pow(2,15));
    B7 = (((unsigned long)UP - B3)*(50000>>oss));

    if(B7 < 0x80000000)
    {
        p = ((B7*2)/B4);
    }
    else
    {
        p = ((B7/B4)*2);
    }
    X1 = ((p/ pow(2,8)) * (p/ pow(2,8)));
    X1 = ((X1 * 3038)/pow(2,16));
    X2 = ((-7357 * p) / pow(2,16));
    p = (p + ((X1 + X2 + 3791) / pow(2,4)));

    MyMessage.printf("Pressure in Pa = %d \n\r", p);
  
    

    }
  // locker_mutex.unlock(); 
}
 


// main() runs in its own thread in the OS
int main()
{

    SetHigh = 1;
    tick.attach(&flipflop, 2);
   // thread1.start(read_temperature);
   thread2.start(read_pressure);
   
    
    
   
    while (true) 
    {
    thread_sleep_for(5000);
    }
}
