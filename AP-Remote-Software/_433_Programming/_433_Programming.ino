/*
  Example for different sending methods
  
  http://code.google.com/p/rc-switch/
  
*/

#include <RCSwitch.h>

long unsigned int Key1 = 0000001; // Change to individual (random) values
long unsigned int Key2 = 0000002;
long unsigned int Key3 = 0000003;
long unsigned int Key4 = 0000004;


RCSwitch mySwitch = RCSwitch();

void setup() {

  Serial.begin(9600);
  
  // Transmitter is connected to Arduino Pin #10  
  mySwitch.enableTransmit(10);

  // Optional set pulse length.
  // mySwitch.setPulseLength(320);
  
  // Optional set protocol (default is 1, will work for most outlets)
  // mySwitch.setProtocol(2);
  
  // Optional set number of transmission repetitions.
  // mySwitch.setRepeatTransmit(15);
  
}

void loop() {

  
  /* Same switch as above, but using decimal code */
  mySwitch.send(Key1, 24); // use key 1 to key 4 to program remote control (key A to D)
  delay(100);  

  
}
