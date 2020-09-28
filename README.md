# Seatalk-Autopilot-Remote-Control

This repository shows how to build a Seatalk Autopilot Remote Control device based on an Arduino Pro Micro and a simple 433 MHz KeyFob. It has been tested successfully with different Raymarine autopilots.

My special thanks to users of www.segeln-forum.de for testing.

![Autopilot Remote](https://github.com/AK-Homberger/Seatalk-Autopilot-Remote-Control/blob/master/IMG_0857.JPG)

![Autopilot Schematic](https://github.com/AK-Homberger/Seatalk-Autopilot-Remote-Control/blob/master/Remote%20Pilot%20Schematics.png)

# Hardware
The circuit diagram and the PCB layout are shown in the hardware section of this repository.

I made the PCB available at aisler.net (https://aisler.net/p/LCDCUVMF). Minimum number is three pieces for about 24 EUR. So it makes sense to buy once and share with others interested in the remote control.

The circuit implements mainly an interface between the Seatalk bus (12 Volt) and the Arduino (5 Volt).
The easiest way was to use the 74LS07 open collector driver device. This is much easier than most of the other implementations I have seen so far. The PCB also contains the voltage regulator (12-14 Volt from Seatalk bus to 5 Volt for the Arduino) and a 433 MHz receiver. Please select/buy the right ProMicro device (5 Volt). There are also 3.3 Volt devices available. But the 3.3 Volt devices will not work.

Please also select a high quality receiver which fits to the PCB design (RXB6, Superhet). As antenna please use a short wire (e.g. 17,3 cm).

The PCB also allows the connection of a standard OLED device (SSD1306) to show the current apparent wind speed. But this is optional.

The voltage regulator (7805) may get quite hot (90 mA * 9 Volt = 0,81 Watt). This is usually not a problem, because it within the specification of the 7805 (< 1,4 Watt).

An alternative which is not getting hot is the following replacement which has the same pin layout as the 7805: https://www.digikey.de/product-detail/en/w-rth-elektronik/173950578/732-8243-5-ND/5725367?cur=EUR&lang=en

A standard 4 button 433 MHz KeyFob is used to send (+1 / -1 degree) and (+10 / -10 degrees) course changes to the autopilot.
The used Seatalk codes are derived from Thomas Knauf (http://www.thomasknauf.de/rap/seatalk2.htm ). Many thanks for this excellent piece of work.

The software is sending a beep to the Raymarine devices (Seatalk alarm function). This helps to get a feedback regarding the recognition of of a pressed key. The LED on the PCB blinks in addition if keys are successfully recognised. This should also help with trouble shooting. It is also possible to connect an active 5 Volt buzzer to pin 20 (A2) and GND of the ProMicro.

Since version 1.1 it supports now also Auto and Standby remote control. You have to use a 6 button KeyFob or an additional 2 or 4 button remote for controling Auto/Standby function.

# Caution: Lighthouse III does not react on Seatalk Alarm 
MFDs with Lighthose II will react on the Seatalk alarm function with a short beep. Unfortunately, Lighthouse III is ignoring this alarm. If you prefer to use Lighthouse III then please connect an additonal buzzer to pin 20 of the ProMicro.

# Software
The Seatalk communication is a 9 bit serial protocol. This is not directly supported from Arduino. But I found a working 9 bit hardware serial implementation. For convenience, I have added the working source code here, because there are different versions available (in github and elsewhere).

Caution: The 9 bit hardware serial implementaion is depending on the version of the Ardunio IDE and the ProMicro board information. IDE 1.6.13  and board version 1.6.15 are working with the files in "NewHWSworking". Later versions (e.g. 1.8.10) changed the hardware serial implementation and the 9 bit changes will work work with files in "HardwareSerial9bit".

For newer IDE versions the three files from "HardwareSerial9bit" (or "NewHWSworking" for older IDE) have to be copied into the Arduino directory on your local PC e.g.: "C:\Program Files (x86)\Arduino\hardware\arduino\avr\cores\arduino".

As Arduino board for the ProMicro you can either use the standard Arduino "Leonardo" board information or the ProMicro  SparkFun implementation: https://learn.sparkfun.com/tutorials/pro-micro--fio-v3-hookup-guide . Both will work.

Make sure that you install the ProMicro Board Addon using the Board Manager as described in the hookup guide: https://learn.sparkfun.com/tutorials/pro-micro--fio-v3-hookup-guide#windows_boardaddon

The software is using several additional Arduino libraries (RCSwitch, Adafruit_GFX, Adafruit_SSD1306). These libraries have to be downloaded and installed. The easiest way is to add the libraries with the IDE Library Manager.

- https://github.com/sui77/rc-switch
- https://github.com/adafruit/Adafruit-GFX-Library
- https://github.com/adafruit/Adafruit_SSD1306

# Programming the 433 MHz KeyFob

You should change the four codes in the software to individual random numbers. The same codes have to be copied to the "_433_Programming_New.ino". Only for programming you will also need a 433 MHz sender connected to the ProMicro.
I used a ProMicro on a "bread board" to program the codes. 

![Connections](https://github.com/AK-Homberger/Seatalk-Autopilot-Remote-Control/blob/master/Programming%20connections.JPG)

Send each code individually and programm the keys of the remote according to the programming description of your KeyFob. Press "Send" button in serial console for next key. 

Key 1 = button A = -1 degree

Key 2 = button B =  +0 degree

Key 3 = button C = -10 degrees

Key 4 = button D = +10 degrees

If you use Auto/Standby function you also have to program the two additional buttons accordingly. After programming you can check directly if codes are programmed correctly. Just press the key on the remote and check if the shown value is correct.

Have fun with the remote control.

Please be aware that this repository is only for educational purpose, to learn how to use the Seatalk protocol. Use it on your own risk and do not use it for critical systems in real life environments.

# Partlist:

- J1 Connector, AKL 101-03, 3-pin, 5,08 mm [Link](https://www.reichelt.com/index.html?ACTION=446&LA=0&nbc=1&q=akl%20101-03)
- J2 RXB6 433 Mhz receiver [Link](https://www.makershop.de/en/module/funk/rxb6-433mhz-antenne/)
- J4 Arduino ProMicro, 5 Volt [Link](https://eckstein-shop.de/HIMALAYA-basic-Pro-micro-5V-16MHz-Arduino-mini-Leonardo-compatible-board ) 
- U1 7805  (alternative: 173950578 MagI3C DC-DC Power Module) [Link](https://www.reichelt.com/de/en/voltage-regulator-up-to-1-5-a-positive-to-220--a-7805-p23443.html?&trstct=pos_0&nbc=1)
- U2 7407N (alternative: 74LS07) [Link](https://www.reichelt.com/de/en/index.html?ACTION=446&LA=446&nbc=1&q=7407n)
- D1 Zenerdiode 5,1 Volt (0,5 Watt) [Link](https://www.reichelt.com/de/en/zener-diode-0-5-w-5-1-v-zf-5-1-p23137.html?&trstct=pos_0&nbc=1)
- D2 LED, rot, 3 mm [Link](https://www.reichelt.com/de/en/led-3-mm-low-current-red-led-3mm-2ma-rt-p21626.html?&trstct=pos_8&nbc=1)
- R1 Resistor 10 K, 1/4 watt [Link](https://www.reichelt.com/de/en/metal-film-resistor-10-kohm-0207-250-mw-0-1--arc-mra0207-10k-p237522.html?&trstct=pos_13&nbc=1)
- R2 Resistor 68 K, 1/4 watt [Link](https://www.reichelt.com/de/en/carbon-film-resistor-1-4-w-5-68-kohm-1-4w-68k-p1462.html?&trstct=pos_3&nbc=1)
- R3 Resistor 27 K, 1/4 watt [Link](https://www.reichelt.com/de/en/carbon-film-resistor-1-4-w-5-27-kohm-1-4w-27k-p1392.html?&trstct=pos_0&nbc=1)
- R4 Resistor 270 Ohm, 1/4 watt [Link](https://www.reichelt.com/de/en/carbon-film-resistor-1-4-w-5-270-ohm-1-4w-270-p1390.html?&trstct=pos_13&nbc=1)
- R5 Resistor 10 K, 1/4 watt [Link](https://www.reichelt.com/de/en/metal-film-resistor-10-kohm-0207-250-mw-0-1--arc-mra0207-10k-p237522.html?&trstct=pos_13&nbc=1)

Optional:
OLED display,  096 inch, I2C [Link](https://www.makershop.de/en/display/oled/096-oled-display-blau/)

For programming:
433MHz transmitter [Link](https://www.makershop.de/en/module/funk/superheterodyne-433-mhz-sender/)

KeyFob  [Link](https://www.amazon.de/XCSOURCE-Elektrische-Universal-Garagentor-Fernbedienung-Ersatz-Schl%C3%BCsselanh%C3%A4nger-HS641/dp/B01KJRGMHI/ref=sr_1_9?__mk_de_DE=%C3%85M%C3%85%C5%BD%C3%95%C3%91&dchild=1&keywords=433+mhz+fernbedienung&qid=1601302510&sr=8-9)


# Updates:

- Version 1.4 - 27.07.20: Use of millis() funtion to avoid delay() and timer counter in loop(). Improved detection of 433MHz keys.
- Version 1.3 - 13.12.19: Added definition for certain OLED displays that require this to work properly.
- Version 1.2 - 21.09.19: Added buzzer support on pin 20 (active 5 Volt buzzer works without additional transitor).
- Version 1.1 - 03.08.19: Added support for Auto and Standby (Key 5 and 6).
- Added new programming code (_433_Programming_New.ino) to simplify programming of KeyFob without recompiling and upload for each key.
