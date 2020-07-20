
#include <stdio.h>

#include "lorawan/LoRaWANInterface.h"
#include "lorawan/system/lorawan_data_structures.h"
#include "events/EventQueue.h"
#include "mbed.h"
//#include "mDot.h"
#include "lora_radio_helper.h"
#include <string>
#include <vector>
//

/*
 * Sets up an application dependent transmission timer in ms. Used only when Duty Cycling is off for testing
 */
#define TX_TIMER                        10000

/**
 * Maximum number of events for the event queue.
 * 10 is the safe number for the stack events, however, if application
 * also uses the queue for whatever purposes, this number should be increased.
 */
#define MAX_NUMBER_OF_EVENTS            10

/**
 * Maximum number of retries for CONFIRMED messages before giving up
 */
#define CONFIRMED_MSG_RETRY_COUNTER     3

/**
* This event queue is the global event queue for both the
* application and stack. To conserve memory, the stack is designed to run
* in the same thread as the application and the application is responsible for
* providing an event queue to the stack that will be used for ISR deferment as
* well as application information event queuing.
*/
static EventQueue ev_queue(MAX_NUMBER_OF_EVENTS *EVENTS_EVENT_SIZE);

/**
 * Event handler.
 *
 * This will be passed to the LoRaWAN stack to queue events for the
 * application which in turn drive the application.
 */
static void lora_event_handler(lorawan_event_t event);

/**
 * Constructing Mbed LoRaWANInterface and passing it the radio object from lora_radio_helper.
 */
static LoRaWANInterface lorawan(radio);

/**
 * Application specific callbacks
 */
static lorawan_app_callbacks_t callbacks;
//
#define sabitNode 1
//sabit nod verisi SX1272_LoRaRadio.h da tanımlı
/*struct rxstruct
{
    uint8_t id;
    uint8_t x;
    uint8_t y;
    uint8_t retcode;
    uint8_t rssi;
}__attribute__((packed)) rxdata;*/
// sensor veri paketi
struct acc
{
    uint8_t heart_rate;
    float x;
    float y;
    float z;
}__attribute__((packed)) acc;
//Ana paket Struct
struct packet
{
    uint8_t id;
    struct acc acc_data;
    //struct rxstruct rx_data;
    uint8_t amkbuffer[30];
} __attribute__((packed)) pkt;
//sen packet
int send_packet(struct packet pkt)
{
    radio.send((uint8_t*)&pkt, (uint8_t)sizeof pkt);
    return 0;
}

int main(){
    //RX buffer    
    uint8_t rx_buffer[30];//y x id
    int rssi;
    //paket verisi
    pkt = {0};
    pkt.id = 2;
    pkt.acc_data = {60,12.2, 5.3, 3.1415};
    
    lorawan.initialize(&ev_queue);
    //
    if(MBED_CONF_APP_LORA_RADIO == SX1272)
    {
        printf("sx1272\n");
    }
    //radio.init_radio(&ev_queue);
    while(1)
    {
    //RX
    radio.lock();
    radio.set_channel(868100000);
    /*set_rx_config(radio_modems_t modem, uint32_t bandwidth,
                         uint32_t datarate, uint8_t coderate,
                         uint32_t bandwidth_afc, uint16_t preamble_len,
                         uint16_t symb_timeout, bool fix_len,
                         uint8_t payload_len,
                         bool crc_on, bool freq_hop_on, uint8_t hop_period,
                         bool iq_inverted, bool rx_continuous)*/
    radio.set_rx_config(MODEM_LORA,0,7,4,0,MBED_CONF_LORA_UPLINK_PREAMBLE_LENGTH,10,0,100,1,0,0,0,0);    
    radio.unlock();
    
    radio.lock();  
    //radio.receive(); 
    radio.receive(1);
    wait(0.5);
    //radio.read_register(0, rx_buffer, 3);//radio.read_fifo(rx_buffer,3);//=radio.read_register(0, buffer, size);
    radio.readRegPublic(pkt.amkbuffer,30);
    //rssi=radio.get_rssi(MODEM_LORA);
    //pkt.amkbuffer[3]=radio.getrssipublic(MODEM_LORA);
    //pkt.rx_data={rx_buffer[0],rx_buffer[1],rx_buffer[2],rx_buffer[3],rssi};
    radio.unlock(); 
    //TX
    radio.lock();
    radio.set_channel(867500000);
    /*set_tx_config(radio_modems_t modem, int8_t power,
                                     uint32_t fdev, uint32_t bandwidth,
                                     uint32_t datarate, uint8_t coderate,
                                     uint16_t preamble_len, bool fix_len,
                                     bool crc_on, bool freq_hop_on,
                                     uint8_t hop_period, bool iq_inverted,
                                     uint32_t timeout)*/
    radio.set_tx_config(MODEM_LORA,0,0,0,7,4,MBED_CONF_LORA_UPLINK_PREAMBLE_LENGTH,0,1,0,0,0,3000);
    radio.set_max_payload_length(MODEM_LORA,100);
    //uint8_t buffer[] = "Dal ruzgari affeder ama kirilmistir bir kere.";
    radio.unlock();
    
    radio.lock();
    uint8_t hr=60;
    float x= 12.2;
    float y= 3.9;
    float z= 6.7;
       
        if(hr==70){hr=60; x=12.2; y=5.3; z=3.1415;}  
        pkt.acc_data = {hr,x,y,z};
        
        send_packet(pkt);//=radio.send((uint8_t*)&pkt, (uint8_t)sizeof pkt);
        hr++; x+=1.0;y+=0.25;z+=0.5;
        printf("sent\n");
        //radio.set_low_power_mode(0);
    }
    radio.unlock();
    return 0;
}