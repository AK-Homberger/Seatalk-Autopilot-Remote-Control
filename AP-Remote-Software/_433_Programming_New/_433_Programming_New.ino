/*
  Code to program 433 MHz KeyFob
  
  Transmitter must be connected to Arduino Pin #10

*/

#include <RCSwitch.h>

long unsigned int Key[] = {0000001, 0000002, 0000003, 0000004 };
int Keys = 4;


RCSwitch mySwitch = RCSwitch();

void setup() {

  Serial.begin(9600);

  delay(1000);
  Serial.println("Press 'Send' to start programming!");

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
  int i;
  char buffer[40];

  while (!Serial.available()) delay(10);
  while (Serial.available()) Serial.read();

  Serial.println();

  for (i = 0; i < Keys; i++) {

    sprintf(buffer, "Sending Key: %d, Value: %07d", i + 1, Key[i]);
    Serial.println(buffer);
    Serial.println("Press 'Send' for next Key.");

    while (!Serial.available()) {
      mySwitch.send(Key[i], 24);
      Serial.print(".");
      delay(100);
    }
    Serial.println();
    while (Serial.available()) Serial.read();
  }
  Serial.println("Press 'Send' to start programming!");
}
