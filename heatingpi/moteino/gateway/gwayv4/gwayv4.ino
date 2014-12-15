// Moteino Library and code by Felix Rusu - felix@lowpowerlab.com
// Get the RFM69 and SPIFlash library at: https://github.com/LowPowerLab/
// -------------------------------------------------------------------
// V4 Gareth Naylor
// Gateway Program with connected DALLAS DS18B20
// This script receives temperature readings via radio
// from other Moteino's (running a different script)
// and sends upstream to connected Pi
// This gateway also has a temp sensor attached
// A local temp reading is taken when "z" is received as input on Serial connection
// All readings and received data are sent to the 
// serial port in the format:-
// zone:temp
// where zone is a integer that defines the sender
// and temp is the temperature in Celcius to 2 dp but *100
// i.e 3:1234 means a temp of 12.34 C from ID 3
// ------------------------------------------------------------------
//

// import libraries needed

#include <RFM69.h>                       // Moteino Radio Libs
#include <SPI.h>                         // SPI lib for Flash comms
#include <SPIFlash.h>                    // FLASH libs
#include <OneWire.h>                     // One-wire libs

// defines

#define DEBUG 0                          // set to 1 for additional serial port output

#define ZONE  3                          // study

#define TPIN 4                           // Digital Pin4 PD4 D4 connected to DS18B20
#define NODEID        0                  // this is "0" because this is the GATEWAY moteino
#define NETWORKID     204                // the same on all nodes that talk to each other
#define FREQUENCY     RF69_433MHZ        // Transmit Frequency for UK
#define ENCRYPTKEY    "16 char String"   // exactly the same 16 characters/bytes on all nodes!
#define ACK_TIME      30                 // max # of ms to wait for an ack
#define SERIAL_BAUD   115200             // baud rate on serial port.
#define LED           9                  // Moteinos have LEDs on D9
#define FLASH_SS      8                  // and FLASH SS on D8

// Setup Rasio, One-wire and Timer

RFM69 radio;                             // Setup an instance of the Radio
SPIFlash flash(FLASH_SS, 0xEF30);        // EF30 for 4mbit  Windbond chip (W25X40CL)
bool promiscuousMode = false;            // set to 'true' to sniff all packets on the same network
OneWire ds(TPIN);                        // One-wire setup for Dall Temp sensor

// Setup Run Once

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(10);
  radio.initialize(FREQUENCY,NODEID,NETWORKID);
#ifdef IS_RFM69HW
  radio.setHighPower();                   // only for RFM69HW!
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

// Main Loop

byte ackCount=0;
void loop() {

  // process any serial input to gateway from Pi
  if (Serial.available() > 0)
  {
    char input = Serial.read();
    if (input == 'r') //d=dump all register values
      radio.readAllRegs();
    if (input == 'E') //E=enable encryption
      radio.encrypt(ENCRYPTKEY);
    if (input == 'e') //e=disable encryption
      radio.encrypt(null);
        
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
    if (input == 'z')  // read from DALLAS temp sensor
    {
      GetDallasTemp();
    }
  }
  
  // Process any DATA received from Remote Moteinos and send upstream on serial port.

  if (radio.receiveDone())
  {
    // Send recieved data up to the Pi on the serial Port. 
    //for (byte i = 0; i < radio.DATALEN; i++)
    //  Serial.print((char)radio.DATA[i]);
    
   for (byte i = 0; i < radio.DATALEN; i++)
     Serial.print((char)radio.DATA[i]);
    
    
    //if (DEBUG) Serial.print("   [RX_RSSI:");Serial.print(radio.RSSI);Serial.print("]");
    
    if (radio.ACKRequested())                // Ensure an Ack is sent back to sender
    {
      byte theNodeID = radio.SENDERID;
      radio.sendACK();
      if (DEBUG) Serial.print(" - ACK sent.");

      // When a node requests an ACK, respond to the ACK
      // and also send a packet requesting an ACK (every 3rd one only)
      // This way both TX/RX NODE functions are tested on 1 end at the GATEWAY
      //if (ackCount++%3==0)
      //{
       // Serial.print(" Pinging node ");
        //Serial.print(theNodeID);
        //Serial.print(" - ACK...");
        //delay(3); //need this when sending right after reception .. ?
        //if (radio.sendWithRetry(theNodeID, "ACK TEST", 8, 0))  // 0 = only 1 attempt, no retries
          //Serial.print("ok!");
        //else Serial.print("nothing");
      //}
    }
    Serial.println();
    Blink(LED,3);
  }
  
// End of Loop
 
}

// Function to BLINK LED on Moteino
void Blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN,HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN,LOW);
}

// Function to get a temperature reading from the sensor
void GetDallasTemp()
{
  // OneWire variables
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float TEMP;
  long temp;
  
  
  // Check for an attached DS18B20 device
  // if none found return
  //if ( !ds.search(addr)) {
  //  if (DEBUG) Serial.println("No more addresses.");
  //  Serial.println();
  //  ds.reset_search();
  //  delay(250);
  //  return;
  //  }

  ds.search(addr);           // find the DS device

  // Use One Wire Bus to read the temperature
  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end
  
  delay(800);              // maybe 750ms is enough, maybe not
  
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
   if (DEBUG==2) Serial.print(" RAW=");
   if (DEBUG==2) Serial.print(raw);
   TEMP = (float)raw / 16.0;
   if (DEBUG==2) Serial.print(" TEMP=");
   if (DEBUG==2) Serial.print(TEMP);
       
   // Handle Results and send upstream to serial port
   temp = long(TEMP*100);             // Scale by 100 
   if ( temp != 0 )
   {
    Serial.println();
    if (DEBUG) Serial.print("Temperature is ");
    Serial.print(ZONE);
    Serial.print(":");
    Serial.print(temp);
    }  
  
}



