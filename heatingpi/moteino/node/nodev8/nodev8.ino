// Sample RFM69 receiver/gateway sketch, with ACK and optional encryption
// Passes through any wireless received messages to the serial port & responds to ACKs
// It also looks for an onboard FLASH chip, if present
// Library and code by Felix Rusu - felix@lowpowerlab.com
// Get the RFM69 and SPIFlash library at: https://github.com/LowPowerLab/
// V8 Gareth Naylor
// Gateway Program with connected DALLAS DS18B20
// temp sensor

#include <RFM69.h>
#include <SPI.h>
#include <SPIFlash.h>
#include <OneWire.h>
#include <LowPower.h>

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
#define ENCRYPTKEY    "16 Char String" //exactly the same 16 characters/bytes on all nodes!
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

// One-wire setup
OneWire ds(TPIN);
float celsius;
long temp;


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
  
// OneWire variables
byte i;
byte present = 0;
byte type_s;
byte data[12];
byte addr[8];
char payload[8];
float celcius;
int sendSize;

  
// Check for an attached DS18B20 device
// if none found return
if ( !ds.search(addr)) {
  if (DEBUG) Serial.println("No more addresses.");
  Serial.println();
  ds.reset_search();
  delay(250);
  return;
  }
 
// Use One Wire Bus to read the temperature
ds.reset();
ds.select(addr);
ds.write(0x44, 1);        // start conversion, with parasite power on at the end
  
delay(1000);     // maybe 750ms is enough, maybe not
  
present = ds.reset();
ds.select(addr);    
ds.write(0xBE);         // Read Scratchpad

if (DEBUG) Serial.print("  Data = ");
if (DEBUG) Serial.print(present, HEX);
if (DEBUG) Serial.print(" ");
for ( i = 0; i < 9; i++) {           // we need 9 bytes
  data[i] = ds.read();
  if (DEBUG) Serial.print(data[i], HEX);
  if (DEBUG) Serial.print(" ");
  }
  if (DEBUG) Serial.print(" CRC=");
  if (DEBUG) Serial.print(OneWire::crc8(data, 8), HEX);
  if (DEBUG) Serial.println();

// Convert the data to actual temperature
int16_t raw = (data[1] << 8) | data[0];
if (type_s) {
  raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
    // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
 } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
      else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
      else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
      // default is 12 bit resolution, 750 ms conversion time
 }
  
  celsius = (float)raw / 16.0;
  celsius = celsius*100;
  temp = long(celsius);
  if (DEBUG) Serial.println();
  if (DEBUG) Serial.print("Temperature is ");
  if (DEBUG) Serial.print(ZONE);
  if (DEBUG) Serial.print(":");
  if (DEBUG) Serial.print(celsius); 
  // build comms payload to send via radio
  // format nodeid:temp
  // and set sendSize to the correct (non zero) value?
  sprintf(payload,"%d:%.4d",NODEID,celcius);
  sendSize=sizeof(payload);
  
  if (sendSize != 0)
  {
    // Send data over Radio to gateway
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
   
    // put Moteino to sleep for loop limit * 8 seconds - params for ATMega328p
    // for 10 mins use 75
    // for 5 mins use ~38
    // for 15 mins use 112
    for(byte c = 0; c < 75; c++)
      LowPower.powerDown(SLEEP_8S,ADC_OFF, BOD_OFF);
      
  //process any serial input
  if (Serial.available() > 0)
  {
    char input = Serial.read();
    if (input == 'r') //d=dump all register values
      radio.readAllRegs();
    if (input == 'E') //E=enable encryption
      radio.encrypt(ENCRYPTKEY);
    if (input == 'e') //e=disable encryption
      radio.encrypt(null);
    if (input == 'p')
    {
      promiscuousMode = !promiscuousMode;
      radio.promiscuous(promiscuousMode);
      Serial.print("Promiscuous mode ");Serial.println(promiscuousMode ? "on" : "off");
    }
  
    if (input == 'i')
    {
      Serial.print("DeviceID: ");
      word jedecid = flash.readDeviceId();
      Serial.println(jedecid, HEX);
    }
    if (input == 't')
    {
      byte temperature =  radio.readTemperature(-1); // -1 = user cal factor, adjust for correct ambient
      byte fTemp = 1.8 * temperature + 32; // 9/5=1.8
      Serial.print( "Radio Temp is ");
      Serial.print(temperature);
      Serial.print("C, ");
      Serial.print(fTemp); //converting to F loses some resolution, obvious when C is on edge between 2 values (ie 26C=78F, 27C=80F)
      Serial.println('F');
    }
  }

  if (radio.receiveDone())
  {
    Serial.print('[');Serial.print(radio.SENDERID, DEC);Serial.print("] ");
    if (promiscuousMode)
    {
      Serial.print("to [");Serial.print(radio.TARGETID, DEC);Serial.print("] ");
    }
    for (byte i = 0; i < radio.DATALEN; i++)
      Serial.print((char)radio.DATA[i]);
    Serial.print("   [RX_RSSI:");Serial.print(radio.RSSI);Serial.print("]");
    
    if (radio.ACKRequested())
    {
      byte theNodeID = radio.SENDERID;
      radio.sendACK();
      Serial.print(" - ACK sent.");

      // When a node requests an ACK, respond to the ACK
      // and also send a packet requesting an ACK (every 3rd one only)
      // This way both TX/RX NODE functions are tested on 1 end at the GATEWAY
      if (ackCount++%3==0)
      {
        Serial.print(" Pinging node ");
        Serial.print(theNodeID);
        Serial.print(" - ACK...");
        delay(3); //need this when sending right after reception .. ?
        if (radio.sendWithRetry(theNodeID, "ACK TEST", 8, 0))  // 0 = only 1 attempt, no retries
          Serial.print("ok!");
        else Serial.print("nothing");
      }
    }
    Serial.println();
    Blink(LED,3);
  }
}

void Blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN,HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN,LOW);
}
