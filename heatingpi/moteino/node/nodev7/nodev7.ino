// V5 8-10-2014
// Moteino sketch to periodically send the temp measured by the radio
// to a gateway node.
//
// Citations:
// Moteino Library and code by Felix Rusu - felix@lowpowerlab.com
// Get the RFM69 and SPIFlash library at: https://github.com/LowPowerLab/

// include necessary libraries
#include <RFM69.h>
#include <SPI.h>
//#include <SPIFlash.h>
#include "LowPower.h"

// RF69 definitions for the radio
#define NODEID        2    //unique for each node on same network
#define NETWORKID     204  //the same on all nodes that talk to each other
#define GATEWAYID     1
#define FREQUENCY   RF69_433MHZ
#define ENCRYPTKEY    "16 Char String"" //exactly the same 16 characters/bytes on all nodes!
#define ACK_TIME      100 // max # of ms to wait for an ack
#define LED           9 // Moteinos have LEDs on D9
#define DELAY 		900	// delay of 15 minutes expressed as seconds

// Common setup
#define SERIAL_BAUD   115200


// radio setup
int TRANSMITPERIOD = 300; //transmit a packet to gateway so often (in ms)
char payload[]="0:23.50";  // set with dummy data nodeid:temp.xx
byte sendSize=0;
boolean requestACK = false;
RFM69 radio;


void setup() {
  // radio setup and variables
  Serial.begin(SERIAL_BAUD);
  radio.initialize(FREQUENCY,NODEID,NETWORKID);
  radio.encrypt(ENCRYPTKEY);
  float celsius
}

void loop() {
 
  celsius = (float)raw / 16.0;
  Serial.print("  Temperature = ");
  Serial.print(celsius);
  Serial.print(" Celsius, ");
  
  // build comms payload to send via radio
  // format nodeid:temp
  // and set sendSize to the correct (non zero) value?
  sprintf(payload,"%d:%.2f",NODEID,celcius);
  sendSize=sizeof(payload);

  //check for any received packets - send to serial port for debug
  if (radio.receiveDone())
  {
    Serial.print('[');Serial.print(radio.SENDERID, DEC);Serial.print("] ");
    for (byte i = 0; i < radio.DATALEN; i++)
      Serial.print((char)radio.DATA[i]);
    Serial.print("   [RX_RSSI:");Serial.print(radio.RSSI);Serial.print("]");

    if (radio.ACKRequested())
    {
      radio.sendACK();
      Serial.print(" - ACK sent");
    }
    Blink(LED,5);
    Serial.println();
  }

  //if we have valid data then send
 
 if (sendSize != 0)
  {
    Serial.print("Sending[");
    Serial.print(sendSize);
    Serial.print("]: ");
    for(byte i = 0; i < sendSize; i++)
      Serial.print((char)payload[i]);

    if (radio.sendWithRetry(GATEWAYID, payload, sendSize))
     Serial.print(" ok!");
    else Serial.print(" nothing...");

    sendSize = 0
    Serial.println();
    Blink(LED,3);
  }
  
   // GO to SLEEP FOR A PERIOD OF TIME
   // PUT RADIO into sleep and then MICRCONCONTROLLER INTO HYBERNATION
    radio.sleep()	// put radio to sleep
    // put Moteino to sleep for 900 seconds - params for ATMega328p
    // change the SLEEP_900S string for other delays
    LowPower.powerDown(SLEEP_900S,ADC_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, SPI_OFF, USART0_OFF, TWI_OFF);
}

void Blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN,HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN,LOW);
}
