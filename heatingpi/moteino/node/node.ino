// V4 8-10-2014
// Moteino sketch to periodically send the temp recevied from an
// attached DS18B20 sensor to a gateway node. 
//
// Citations:
// Moteino Library and code by Felix Rusu - felix@lowpowerlab.com
// Get the RFM69 and SPIFlash library at: https://github.com/LowPowerLab/

// OneWire DS18S20, DS18B20, DS1822 Temperature
// http://www.pjrc.com/teensy/td_libs_OneWire.html
// http://milesburton.com/Dallas_Temperature_Control_Library
//
// include necessary libraries
//#include <OneWire.h>
#include <RFM69.h>
#include <SPI.h>
#include <SPIFlash.h>
#include <LowPower.h>
#include <float.h>;

// RF69 definitions for the radio
#define NODEID        2    //unique for each node on same network
#define NETWORKID     204  //the same on all nodes that talk to each other
#define GATEWAYID     1
#define FREQUENCY   RF69_433MHZ
#define ENCRYPTKEY    "CockRobinWood204" //exactly the same 16 characters/bytes on all nodes!
#define ACK_TIME      100 // max # of ms to wait for an ack
#define LED           9 // Moteinos have LEDs on D9
#define DELAY 		900	// delay of 15 minutes expressed as seconds
#define SPIN          14    // temp sensor input pin

// Common setup
#define SERIAL_BAUD   115200


// radio setup
int TRANSMITPERIOD = 300; //transmit a packet to gateway so often (in ms)
char payload[10];  // 
byte sendSize=0;
boolean requestACK = false; //set with dummy data nodeid:temp.xx
RFM69 radio;

// One-wire setup
//OneWire ds(SPIN);  // on pin 10 -set to correct pin - (a 4.7K resistor is necessary)
float celsius;
long temp;

void setup() {
  // radio setup and variables
  Serial.begin(SERIAL_BAUD);
  radio.initialize(FREQUENCY,NODEID,NETWORKID);
  radio.encrypt(ENCRYPTKEY);   
}

void loop() {
  
 // OneWire variables
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  
  
  // Check for an attached DS18B20 device
  // if none found return
//  if ( !ds.search(addr)) {
//    Serial.println("No more addresses.");
//    Serial.println();
//    ds.reset_search();
//    delay(250);
//    return;
//  }
  
  // Use One Wire Bus to read the temperature
//  ds.reset();
//  ds.select(addr);
//  ds.write(0x44, 1);        // start conversion, with parasite power on at the end
  
//  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.
  
//  present = ds.reset();
//  ds.select(addr);    
//  ds.write(0xBE);         // Read Scratchpad

//  Serial.print("  Data = ");
//  Serial.print(present, HEX);
//  Serial.print(" ");
//  for ( i = 0; i < 9; i++) {           // we need 9 bytes
//    data[i] = ds.read();
//    Serial.print(data[i], HEX);
//    Serial.print(" ");
//  }
//  Serial.print(" CRC=");
//  Serial.print(OneWire::crc8(data, 8), HEX);
//  Serial.println();

 // Convert the data to actual temperature
//  int16_t raw = (data[1] << 8) | data[0];
//  if (type_s) {
//    raw = raw << 3; // 9 bit resolution default
//    if (data[7] == 0x10) {
    // "count remain" gives full 12 bit resolution
//     raw = (raw & 0xFFF0) + 12 - data[6];
//    }
//  } else {
//    byte cfg = (data[4] & 0x60);
//    // at lower res, the low bits are undefined, so let's zero them
//    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
//    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
//    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    // default is 12 bit resolution, 750 ms conversion time
//  }
//  celsius = (float)raw / 16.0;
  celsius=123.45;
  celsius = celsius*100;
  temp = long(celsius);
     
  // format nodeid:temp
  // and set sendSize to the correct (non zero) value?
  sprintf(payload,"%02d:%4d\0", NODEID, temp ); 
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

    sendSize = 0;
    Serial.println();
    Blink(LED,3);
  }
  
   // GO to SLEEP FOR A PERIOD OF TIME
   // PUT RADIO into sleep and then MICRCONCONTROLLER INTO HYBERNATION
   radio.sleep();	// put radio to sleep
    // put Moteino to sleep for 900 seconds - params for ATMega328p
    
    LowPower.powerDown(SLEEP_8S,ADC_OFF,BOD_OFF);
 //   LowPower.powerDown(SLEEP_8S,ADC_OFF,BOD_OFF);
 //   LowPower.powerDown(SLEEP_8S,ADC_OFF,BOD_OFF);
 //   LowPower.powerDown(SLEEP_8S,ADC_OFF,BOD_OFF);
    
}

void Blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN,HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN,LOW);
}
