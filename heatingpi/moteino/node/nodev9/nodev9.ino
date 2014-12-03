// Sample RFM69 receiver/gateway sketch, with ACK and optional encryption
// Passes through any wireless received messages to the serial port & responds to ACKs
// It also looks for an onboard FLASH chip, if present
// Library and code by Felix Rusu - felix@lowpowerlab.com
// Get the RFM69 and SPIFlash library at: https://github.com/LowPowerLab/
// V9 Gareth Naylor
// Node program using internal temp sensor of radio
// 

#include <RFM69.h>
#include <SPI.h>
#include <SPIFlash.h>
#include <LowPower.h>

#define OFFSET 5       // offset in degrees to normalise temp sensor

#define ZONE  1        // Hall
#define DEBUG 0        // set to 1 for additional serial port output
#define TPIN 4             // Digital Pin4 PD4 D4 connected to DS18B20
#define NODEID        1    //unique for each node on same network should match ZONE
#define NETWORKID     204  //the same on all nodes that talk to each other
#define GATEWAYID     0    // the ID of the GATEWAY RX node
//Match frequency to the hardware version of the radio on your Moteino (uncomment one):
#define FREQUENCY     RF69_433MHZ
//#define FREQUENCY     RF69_868MHZ
//#define FREQUENCY     RF69_915MHZ
#define ENCRYPTKEY    "CockRobinWood204" //exactly the same 16 characters/bytes on all nodes!
//#define IS_RFM69HW    //uncomment only for RFM69HW! Leave out if you have RFM69W!
#define ACK_TIME      30 // max # of ms to wait for an ack
#define SERIAL_BAUD   115200

#ifdef __AVR_ATmega1284P__
  #define LED           15 // Moteino MEGAs have LEDs on D15
  #define FLASH_SS      23 // and FLASH SS on D23
#else
  #define LED           9 // Moteinos have LEDs on D9
  #define FLASH_SS      8 // and FLASH SS on D8
#endif

RFM69 radio;
SPIFlash flash(FLASH_SS, 0xEF30); //EF30 for 4mbit  Windbond chip (W25X40CL)
bool promiscuousMode = false; //set to 'true' to sniff all packets on the same network

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(10);
  radio.initialize(FREQUENCY,NODEID,NETWORKID);
#ifdef IS_RFM69HW
  radio.setHighPower(); //only for RFM69HW!
#endif
  radio.encrypt(ENCRYPTKEY);
  radio.promiscuous(promiscuousMode);
  char buff[50];
  sprintf(buff, "\nListening at %d Mhz...", FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915);
  if (DEBUG) Serial.println(buff);
  if (flash.initialize())
  {
    if (DEBUG) Serial.print("SPI Flash Init OK ... UniqueID (MAC): ");
    flash.readUniqueId();
    for (byte i=0;i<8;i++)
    {
      if (DEBUG) Serial.print(flash.UNIQUEID[i], HEX);
      if (DEBUG) Serial.print(' ');
    } 
  }
  else
    if (DEBUG) Serial.println("SPI Flash Init FAIL! (is chip present?)");
}

byte ackCount=0;
void loop() {
  
  char payload[7];
  long celcius;
  int sendSize;

// Read the temp of the radio

  byte temperature =  radio.readTemperature(OFFSET); // adjust for correct ambient temp
  
  celcius=long(temperature)*100;

  if (DEBUG) Serial.print( "Radio Temp is ");
  if (DEBUG) Serial.print(temperature);

  if (DEBUG) Serial.println();
  if (DEBUG) Serial.print("Temperature is ");
  if (DEBUG) Serial.print(ZONE);
  if (DEBUG) Serial.print(":");
  if (DEBUG) Serial.print(celcius); 
  if (DEBUG) Serial.print("\n");
  
  // build comms payload to send via radio
  // format nodeid:temp
  // and set sendSize to the correct (non zero) value?

  sprintf(payload,"%d:%.4d",NODEID,celcius);
  sendSize=sizeof(payload);
  
  if (DEBUG) Serial.print("Payload is: ");
  if (DEBUG) Serial.print(payload);
  if (DEBUG) Serial.print("\n");
  
  if (DEBUG) Serial.print("Sendsize is: ");
  if (DEBUG) Serial.print(sendSize);
  if (DEBUG) Serial.print("\n");
  
  if (sendSize != 0)
  {
    // Send data over Radio to gateway
    if (radio.sendWithRetry(GATEWAYID, payload, sendSize))
    Serial.print("ok!\n");
    else Serial.print(" nothing...\n");
    sendSize = 0;
    Serial.println();
    Blink(LED,3);
  }
  
   // GO to SLEEP FOR A PERIOD OF TIME
   // PUT RADIO into sleep and then MICRCONCONTROLLER INTO HYBERNATION
    radio.sleep();	// put radio to sleep
   
    // put Moteino to sleep for loop limit * 8 seconds - params for ATMega328p
    // for 10 mins use 75
    // for 5 mins use ~38
    // for 15 mins use 112
    for(byte c = 0; c < 75; c++)
      LowPower.powerDown(SLEEP_8S,ADC_OFF, BOD_OFF);

 
}

void Blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN,HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN,LOW);
}
