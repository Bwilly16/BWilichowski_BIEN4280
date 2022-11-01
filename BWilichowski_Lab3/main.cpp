/* mbed Microcontroller Library
 * Copyright (c) 2006-2019 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* Author: Ben Wilichowski
    Date: 10/25/2022
    Purpose: Lab 3 - Bluetooth
*/

#include "mbed.h"
#include <cstdint>
#include <events/mbed_events.h>
#include "ble/BLE.h"
#include "ble/Gap.h"
#include <AdvertisingDataSimpleBuilder.h>
#include <ble/gatt/GattCharacteristic.h>
#include <ble/gatt/GattService.h>
#include "att_uuid.h"
#include "DigitalOut.h"

#include <USBSerial.h>

USBSerial ser;

Thread tempthread;
//DigitalOut red(LED2); //red
//DigitalOut blue(LED4);//blue

static events::EventQueue event_queue(16 * EVENTS_EVENT_SIZE);
static ble::scan_duration_t scan_time(ble::millisecond_t(5000));

BLE &bleinit= BLE::Instance();
Gap& gap = bleinit.gap();
GattServer& gattServe = bleinit.gattServer();
GattClient& gattClient = bleinit.gattClient();

int16_t TOUT = 0;

using namespace ble;

/**
 * Event handler struct
 */
struct GattEventHandler : GattServer::EventHandler{

}
struct GapEventHandler : Gap::EventHandler{

    void onScanRequestRecieved(const ScanRequestEvent &event){
        ser.printf("Scan Request Recieved.\n\r");
    }
    void onAdvertisingStart(const AdvertisingStartEvent &event){
        ser.printf("Advertising Started\n\r");
    }
    void onAdvertisingEnd(const AdvertisingEndEvent &event){//unspec,timout,reached,none
        ser.printf("Advertising Stopped\n\r");
        if(event.getStatus() == BLE_ERROR_UNSPECIFIED){
            ser.printf("Advertising Stopped for Unknown Error.\n\r");
        }
        else if (event.getStatus() == BLE_ERROR_TIMEOUT){
            ser.printf("Advertising Stopped due to timeout.\n\r");
        }
        else if (event.getStatus() == BLE_ERROR_LIMIT_REACHED){
            ser.printf("Advertising Stopped due to limit reached.\n\r");
        }
        else if (event.getStatus() == BLE_ERROR_NONE){
            if(event.isConnected()){
                ser.printf("Device is connected!\n\r");
            }
            else{
                ser.printf("Device is NOT connected\n\r");
            }
        }
    
    }
    
    /* 
	 * Implement the functions here that you think you'll need. These are defined in the GAP EventHandler:
     * https://os.mbed.com/docs/mbed-os/v6.6/mbed-os-api-doxy/structble_1_1_gap_1_1_event_handler.html
     */
};


GapEventHandler THE_gap_EvtHandler;


void measure_temp(){
    I2C sensor_bus(I2C_SDA1, I2C_SCL1);

    const int readaddr = 0xEF;
    const int writeaddr = 0xEE;
    uint8_t whoamiaddr[] = {0xD0};
    int resp=4;

    char readData[] ={0, 0};
    resp = sensor_bus.write(writeaddr, (const char *) whoamiaddr, 1, true);
    
    if(  resp != 0 ){
        ser.printf("I failed to talk at the temp sensor. (Returned: %d)\n\r", resp);            
    }
              
    if( sensor_bus.read(readaddr, readData, 1)  != 0 ){
        ser.printf("I failed to listen to the temp sensor.\n\r");        
    }
    
    ser.printf("Who Am I? %d\n", readData[0] );
    if( readData[0] != 0x55 ){
        ser.printf("Who are are you?\n\r");
    }

    readData[0] = 0x20; // Control Reg 1
    readData[1] = 0x84; // Turn on our temp sensor, and ensure that we read high to low on our values.
    resp = sensor_bus.write(readaddr, readData, 2);    


    uint8_t databuf[2];
    uint8_t subaddr[2];
    while(1){        
        readData[0] = 0xF4; // Control Reg 2
        readData[1] = 0x2E; // Signal a one shot temp reading.
        resp = sensor_bus.write(readaddr, readData, 2);

		thread_sleep_for(5);
        
        subaddr[0] = 0xF6; // MSB Temperature
        sensor_bus.write(writeaddr, (const char *) subaddr, 1, true);
        sensor_bus.read(readaddr, readData, 1);
        databuf[0] = ((uint8_t)readData[0]);

        subaddr[0] = 0xF7; // LSB Temperature
        sensor_bus.write(writeaddr, (const char *) subaddr, 1, true);
        sensor_bus.read(readaddr, readData, 1);
        databuf[1] = readData[0];

        TOUT = (databuf[0]<<8) | databuf[1];
        ser.printf("Uncalibrated temperature: %d\n\r",TOUT);

        // Sleep for a while.
        thread_sleep_for(5000);
    }
}


void on_init_complete(BLE::InitializationCompleteCallbackContext *params){ //callback function defeniton 
    if(params -> error){
        ser.printf("Initialization was NOT complete...\n\r"); //notify terminal that initialization was not successful
    }
    else{
        ser.printf("Initialization was complete!\n\r"); //notify terminal that initialization was successful       
    }
            gap.setEventHandler(&THE_gap_EvtHandler);
            gap.setAdvertisingPayload(
                LEGACY_ADVERTISING_HANDLE,
                AdvertisingDataSimpleBuilder<LEGACY_ADVERTISING_MAX_SIZE>()
                .setFlags()
                .setName("Ben's Chip", true)
                .getAdvertisingData()
                );

      gap.startAdvertising(LEGACY_ADVERTISING_HANDLE);   
      gap.startScan(scan_time);  
}
    

/* Schedule processing of events from the BLE middleware in the event queue. */
void schedule_ble_events(BLE::OnEventsToProcessCallbackContext *context){
    event_queue.call(mbed::Callback<void()>(&context->ble, &BLE::processEvents));
}


int main(){
    DigitalOut i2cbuspull(P1_0); // Pull up i2C. resistor.
    i2cbuspull.write(1);
    DigitalOut sensor_pwr(P0_22); // Supply power to all of the sensors (VCC)
    sensor_pwr.write(1);

    bleinit.onEventsToProcess(schedule_ble_events);
    ble_error_t init(BLE::InitializationCompleteCallbackContext *params);//initialize ble
    bleinit.init(&on_init_complete); //maybe the call back once initialization is complete?

    // This will never return...
    event_queue.dispatch_forever();
}