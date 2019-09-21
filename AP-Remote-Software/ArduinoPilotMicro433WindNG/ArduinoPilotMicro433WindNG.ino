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

// Version 1.2, 21.09.2019, AK-Homberger

#include <avr/pgmspace.h>
#include <RCSwitch.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#define Auto_Standby_Support 0  // Set this to 1 to support Standby and Auto for Key 5 and 6

RCSwitch mySwitch = RCSwitch();

const long unsigned int Key_Minus_1 PROGMEM = 0000001; // Change values to individual values programmed to remote control
const long unsigned int Key_Plus_1 PROGMEM = 0000002;
const long unsigned int Key_Minus_10 PROGMEM = 0000003;
const long unsigned int Key_Plus_10 PROGMEM = 0000004;
const long unsigned int Key_Auto PROGMEM = 0000005;
const long unsigned int Key_Standby PROGMEM = 0000006;

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
long unsigned int timer = 0;
long unsigned int timer1 = 0;
long unsigned int timer2 = 0;

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


void Display(char *string, int size)
{
  display.clearDisplay();
  display.setTextSize(size);
  display.setCursor(0, 0);
  display.println(string);
  display.display();
  timer = 0;
}


int checkWind(char * AWS)     // Receice apparent wind speed from bus
{
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


void setup()
{
  Serial.begin( 9600 );  // Serial out put for function checks with PC
  Serial1.begin( 4800, SERIAL_9N1 );  // Set the Seatalk modus - 9 bit
  Serial1.setTimeout(5);

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


void Beep(void) {
  sendDatagram(ST_BeepOn);
  digitalWrite(20, HIGH);
  delay(150);
  sendDatagram(ST_BeepOff);
  digitalWrite(20, LOW);
}


void loop()
{
  int i;
  char AWS[4] = "";

  timer++; timer1++; timer2++;

  if (timer > 200000 ) {
    Display("---", 7);              // Show --- after about two seconds when no wind data is received
    timer = 0;
  }

  if (timer1 > 300000 ) {
    sendDatagram(ST_BeepOff);       // Additional Beep off after three seconds
    timer1 = 0;
  }


  if (timer2 > 1000000 ) {
    sendDatagram(ST_NMEA_BridgeID); // Send NMEA Seatakl BridgeID every 10 seconds to make Seatalk to Seatalk NG converter happy
    timer2 = 0;
  }


  if (checkWind(AWS) > -1) Display(AWS, 7);

  if (mySwitch.available()) {
    long unsigned int value = mySwitch.getReceivedValue();

    digitalWrite(9, blink);   // LED on/off
    blink = !blink;           // Toggle LED to show received key

    mySwitch.resetAvailable();

    if (value == Key_Minus_1) {
      Display("-1", 7);
      sendDatagram(ST_Minus_1);
      sendDatagram(ST_BeepOn);
      delay(150);
      sendDatagram(ST_BeepOff);
    }

    if (value == Key_Plus_1) {
      Display("+1", 7);
      sendDatagram(ST_Plus_1);
      Beep();
    }

    if (value == Key_Minus_10) {
      Display("-10", 7);
      sendDatagram(ST_Minus_10);
      Beep();
    }

    if (value == Key_Plus_10) {
      Display("+10", 7);
      sendDatagram(ST_Plus_10);
      Beep();
    }

    if ((value == Key_Auto)  && (Auto_Standby_Support == 1)) {
      Display("Auto", 7);
      sendDatagram(ST_Auto);
      Beep();
    }

    if ((value == Key_Standby)  && (Auto_Standby_Support == 1)) {
      Display("Standby", 7);
      sendDatagram(ST_Standby);
      Beep();
    }

    i = 0;
    while (mySwitch.available() && i < 2) {
      mySwitch.resetAvailable();
      delay (150);
      i++;
    }
  }
}
