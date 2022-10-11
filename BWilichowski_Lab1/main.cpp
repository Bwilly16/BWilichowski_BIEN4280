#include "mbed.h"
#include "BWilichowski_binaryutils.h"
#include "MemoryPool.h"
#include "Ticker.h"
#include "USBSerial.h" //light doesnt turn on at all when implemented




#define LEDDIR (uint32_t*) 0x50000514  //In is ON, OUT is OFF
#define LEDSET (uint32_t*) 0x50000508 //Set LED Pin 
#define LEDCLEAR (uint32_t*) 0x5000050c //Clear LED Pin

//Green LED == P0.16
//Red LED == P0.24
//Blue LED == P0.06
typedef struct {
    uint32_t Percentage;
} message_t;

USBSerial MyMessage;

MemoryPool<message_t, 9> MemoryPool1;
Queue<message_t, 9> queue;

Thread Producer1;
Thread ConsumerVanilla;
Thread ConsumerChocolate;

Ticker tick;

    
    float period = 1; //ms
    float dC = 0;
    float time_on = 0;
    float time_off;
    float counter = 0;


void SetOnLed()
{
       
        counter++;
       
        if (counter <= dC)
        {
             setbit(LEDSET, 16);
        }

        else
        {
            setbit(LEDCLEAR, 16); 
        }

        if (counter == 10)
        {
            counter = 0;
        }
}


void Producer(void)
{
    while (true) {
        message_t *Message = MemoryPool1.alloc();

      for(int i = 0; i <= 10; i++)
      {
        MyMessage.printf("Start of counting up\n\r");
        Message-> Percentage = i;
        queue.put(Message);
        thread_sleep_for(100);
       }
        
    for(int i = 10; i >= 0; i--)
      {
        MyMessage.printf("Start of counting down\n\r");
        Message-> Percentage = i;
        queue.put(Message);
        thread_sleep_for(100);
       }

       
      
    }
}

void Vanilla(void)
{
    tick.attach_us(&SetOnLed, 80); //maybe mess with this number?
    thread_sleep_for(100);
      
    while (true) {
        osEvent evt = queue.get(0);
        if (evt.status == osEventMessage) {
            message_t *Message = (message_t *)evt.value.p; //setting PWM to 1 ms
                
            dC = (Message -> Percentage);// * 0.1; //taking the value from 0-9 and multiplying it by 0.1
            time_on = period * dC; //taking the period (1) and multiplying it by the decimal
            time_off = period - time_on;

            MemoryPool1.free(Message);
        }
    }
}


void Chocolate(void)
{

PwmOut led(LED4); //BlueLED

  while (true) {
        osEvent evt = queue.get(0);
        if (evt.status == osEventMessage) {
            message_t *Message = (message_t *)evt.value.p; //setting PWM to 1 ms

        led.period(0.0001f);      
        if(Message -> Percentage == 0)
        {   
            led.pulsewidth(0.00001); 
        }
        else if(Message -> Percentage == 1)
        {   
            led.pulsewidth(0.00002); 
        }
        else if(Message -> Percentage == 2)
        {   
            led.pulsewidth(0.00003); 
        }
        else if(Message -> Percentage == 3)
        {   
            led.pulsewidth(0.00004); 
        }
        else if(Message -> Percentage == 4)
        {   
            led.pulsewidth(0.00005); 
        }
        else if(Message -> Percentage == 5)
        {   
            led.pulsewidth(0.00006); 
        }
        else if(Message -> Percentage == 6)
        {   
            led.pulsewidth(0.00007); 
        }
        else if(Message -> Percentage == 7)
        {   
            led.pulsewidth(0.00008); 
        }
        else if(Message -> Percentage == 8)
        { 
            led.pulsewidth(0.00009); 
        }
        else if(Message -> Percentage == 9)
        { 
          led.pulsewidth(0.0001); 
        }

            MemoryPool1.free(Message);
        }
    }
}

void Strawberry()
{
 while (true) {
        osEvent evt = queue.get(0);
        if (evt.status == osEventMessage) {
            message_t *Message = (message_t *)evt.value.p;

        if(Message -> Percentage == 0)
        {
            //do something for each queue percentage value
        }
        
}

int main(void)
{
    //setbit(LEDDIR, 16); //set the directional as OUT, we want an output to display the light
    //setbit(LEDDIR, 16);
    //setbit(LEDDIR, 24);
    Producer1.start(Producer);//call first thread to add mail to mailbox
    //ConsumerVanilla.start(Vanilla);//use data in the mailbox
    ConsumerChocolate.start(Chocolate);


    while (true) {
      thread_sleep_for(5000);
        }

}

