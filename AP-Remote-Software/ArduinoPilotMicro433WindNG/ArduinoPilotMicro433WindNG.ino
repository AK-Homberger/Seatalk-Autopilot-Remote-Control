/*
  This code is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  This code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

// Version 1.5, 16.12.2020, AK-Homberger

#include <avr/pgmspace.h>
#include <RCSwitch.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define Auto_Standby_Support 0  // Set this to 1 to support Standby and Auto for Key 5 and 6

#define KEY_DELAY 300      // 300 ms break between keys
#define BEEP_DURATION 150  // 150 ms beep time

RCSwitch mySwitch = RCSwitch();

String inputString = "";         // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete

const unsigned long Key_Minus_1 PROGMEM = 1111001; // Change values to individual values programmed to remote control
const unsigned long Key_Plus_1 PROGMEM = 1111002;
const unsigned long Key_Minus_10 PROGMEM = 1111003;
const unsigned long Key_Plus_10 PROGMEM = 1111004;
const unsigned long Key_Auto PROGMEM = 1111005;
const unsigned long Key_Standby PROGMEM = 1111006;

// Seatalk datagrams

const PROGMEM uint16_t ST_NMEA_BridgeID[] =  { 0x190, 0x00, 0xA3 };

const PROGMEM uint16_t ST_Minus_1[] =  { 0x186, 0x21, 0x05, 0xFA };
const PROGMEM uint16_t ST_Minus_10[] = { 0x186, 0x21, 0x06, 0xF9 };
const PROGMEM uint16_t ST_Plus_1[] =   { 0x186, 0x21, 0x07, 0xF8 };
const PROGMEM uint16_t ST_Plus_10[] =  { 0x186, 0x21, 0x08, 0xF7 };
const PROGMEM uint16_t ST_Auto[] =     { 0x186, 0x21, 0x01, 0xFE };
const PROGMEM uint16_t ST_Standby[] =  { 0x186, 0x21, 0x02, 0xFD };


const PROGMEM uint16_t ST_BeepOn[] =  { 0x1A8, 0x53, 0x80, 0x00, 0x00, 0xD3 };
const PROGMEM uint16_t ST_BeepOff[] = { 0x1A8, 0x43, 0x80, 0x00, 0x00, 0xC3 };

boolean blink = true;
unsigned long wind_timer = 0;   // timer for AWS display
unsigned long beep_timer2 = 0;  // timer to stop alarm sound
unsigned long bridge_timer = 0; // timer to send ST Bridge ID every 10 seconds

unsigned long key_time = 0;     // time of last key detected
unsigned long beep_time = 0;    // timer for beep duration
bool beep_status = false;


boolean sendDatagram(const uint16_t data[]) {
  int i = 0; int j = 0;
  boolean ok = true;
  int bytes;
  unsigned int inbyte;
  unsigned int outbyte;

  bytes = (pgm_read_byte_near(data + 1) & 0x0f) + 3; // Messege length is minimum 3, additional bytes in nibble 4

  while (j < 5 ) { // CDMA/CD 5 tries
    while (Serial1.available ()) {  // Wait for silence on the bus
      inbyte = (Serial1.read());
      delay(3);
    }

    ok = true;
    for (i = 0; (i < bytes) & (ok == true); i++) { // Write and listen to detect collisions
      outbyte = pgm_read_word_near(data + i);
      Serial1.write(outbyte);
      delay(3);

      if (Serial1.available ()) {
        inbyte = Serial1.read();  // Not what we sent, collision!

        if (inbyte != outbyte) ok = false;
      }
      else ok = false; // Nothing received
    }

    if ( ok )return ok;

    j++; // Collision detected
    // Serial.println("CD");
    // Display("Collision", 2);
    delay(random(2, 50));  // Random wait for next try
  }
  Display("Send Error", 2);
  return false;
}


void Display(const char *string, int size) {
  display.clearDisplay();
  display.setTextSize(size);
  display.setCursor(0, 0);
  display.println(string);
  display.display();
  wind_timer = millis();
}


// Receive apparent wind speed from bus
int checkWind(char * AWS) {
  unsigned int xx;
  unsigned int y;
  unsigned int inbyte;
  int wind = -1;

  if (Serial1.available ()) {
    inbyte = Serial1.read();
    if (inbyte == 0x111) {    // AWS Seatalk command - See reference from Thomas Knauf
      delay(3);
      inbyte = Serial1.read();
      if (inbyte == 0x01) {   // AWS Setalk command
        delay(3);
        xx = Serial1.read();
        delay(3);
        y = Serial1.read();
        wind = (xx & 0x7f) + (y / 10);  // Wind speed
        if (wind < 100) itoa (wind , AWS, 10);  // Greater 100 must be a receive error
      }
    }
  }
  return wind;
}


void setup() {

  Serial.begin( 9600 );  // Serial out put for function checks with PC
  Serial1.begin( 4800, SERIAL_9N1 );  // Set the Seatalk modus - 9 bit
  Serial1.setTimeout(5);

  // reserve 20 bytes for the inputString:
  inputString.reserve(20);
  
  mySwitch.enableReceive(4);  // RF Receiver on inerrupt 4 => that is pin 7 on Micro

  pinMode(9, OUTPUT);         // LED to show if keys are received
  digitalWrite(9, HIGH);

  pinMode(20, OUTPUT);         // Buzzer to show if keys are received
  digitalWrite(20, LOW);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x64 from Conrad else 3D)
  display.setTextColor(WHITE);
  Display("Start", 4);

  sendDatagram(ST_NMEA_BridgeID);   // Send NMEA Seatakl BridgeID to make Seatalk to Seatalk NG converter happy
}


// Beep on if key received
void BeepOn(void) {

  if (beep_status == true) return;  // Already On

  sendDatagram(ST_BeepOn);
  digitalWrite(20, HIGH);
  //Serial.println("On");
  beep_time = millis();
  beep_status = true;
}


// Beep off after BEEP_TIME
void BeepOff(void) {

  if (beep_status == true && millis() > beep_time + BEEP_DURATION) {
    sendDatagram(ST_BeepOff);
    digitalWrite(20, LOW);
    //Serial.println("Off");
    beep_status = false;
  }
}


/*
  SerialEvent occurs whenever a new data comes in the hardware serial RX. This
  routine is run between each time loop() runs, so using delay inside loop can
  delay response. Multiple bytes of data may be available.
*/
void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    if ((inChar != '\n') && (inChar != '\r')) inputString += inChar;
    // if the incoming character is a newline or CR, set a flag so the main loop can react
    if ((inChar == '\n') || (inChar == '\r')) {
      stringComplete = true;
    }
  }
}


void loop() {
 
  char AWS[4] = "";
  unsigned long value = 0;

  if (millis() > wind_timer + 2000 ) {
    Display("---", 7);              // Show --- after about two seconds when no wind data is received
    wind_timer = millis();
  }

  if (millis() > beep_timer2 + 3000 ) {
    sendDatagram(ST_BeepOff);       // Additional Beep off after three seconds to avoid constant alarm
    beep_timer2 = millis();
  }

  if (millis() > bridge_timer + 10000 ) {
    sendDatagram(ST_NMEA_BridgeID); // Send NMEA Seatakl BridgeID every 10 seconds to make Seatalk to Seatalk NG converter happy
    bridge_timer = millis();
  }

  if (checkWind(AWS) > -1) {
    Display(AWS, 7);
    wind_timer = millis();
  }

  if (mySwitch.available()) {
    value = mySwitch.getReceivedValue();
    mySwitch.resetAvailable();
  }

  serialEvent();  // Read serial to detect command from USB (until Newline or CR)
  
  if (stringComplete) {
        
    // Compare string
    if(inputString == "-1") value = Key_Minus_1;
    if(inputString == "+1") value = Key_Plus_1;
    if(inputString == "-10") value = Key_Minus_10;
    if(inputString == "+10") value = Key_Plus_10;
    if(inputString == "A") value = Key_Auto;
    if(inputString == "S") value = Key_Standby;
    
    // clear the string
    inputString = "";
    stringComplete = false;
  }

  if (value > 0 && millis() > key_time + KEY_DELAY) {

    key_time = millis();      // Remember time of last key received
    digitalWrite(9, blink);   // LED on/off
    blink = !blink;           // Toggle LED to show received key

    if (value == Key_Minus_1) {
      Display("-1", 7);
      sendDatagram(ST_Minus_1);
      BeepOn();
    }

    if (value == Key_Plus_1) {
      Display("+1", 7);
      sendDatagram(ST_Plus_1);
      BeepOn();
    }

    if (value == Key_Minus_10) {
      Display("-10", 7);
      sendDatagram(ST_Minus_10);
      BeepOn();
    }

    if (value == Key_Plus_10) {
      Display("+10", 7);
      sendDatagram(ST_Plus_10);
      BeepOn();
    }

    if ((value == Key_Auto)  && (Auto_Standby_Support == 1)) {
      Display("Auto", 7);
      sendDatagram(ST_Auto);
      BeepOn();
    }

    if ((value == Key_Standby)  && (Auto_Standby_Support == 1)) {
      Display("Standby", 7);
      sendDatagram(ST_Standby);
      BeepOn();
    }
  }
  BeepOff();
}
