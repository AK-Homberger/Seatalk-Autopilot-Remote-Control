# Seatalk-Autopilot-Remote-Control

This repository shows how to build a Seatalk Autopilot Remote Control device based on an Arduino Pro Micro and a simple 433 MHz KeyFob. It has been tested sucessfully with different Raymarine autopilots.

# Hardware
A PCB is available at aisler.net (https://aisler.net/p/LCDCUVMF).

The circuit implements mainly am interface between the Seatalk bus (12 Volt) and the Arduino (5 Volt).
The easiest way was to use the 74LS07 open collector driver device. This is much easier than most of the other implementations I have seen so far. The PCB also contains the voltage regulator (12-14 Volt from Seatalk bus to 5 Volt) and a 433 MhZ receiver.
Please select a high quality receiver which fits to the PCB design (RXB6). As antenna please use a short wire (e.g. 17,3 cm).

The PCB also allows the connection of a standard OLED device (SSD1306) to show the current apparent wind speed. But this is optional.

A standard 4 button 433 MHz KeyFob is used to send (+1 / -1 degree) and (+10 / -10 degrees) course changes to the autopilot.
The used Seatalk codes are drived from Thomas Knauf (http://www.thomasknauf.de/rap/seatalk2.htm ). Many thanks for this excellent piece of work.

# Software
The Seatalk communication is a 9 bit serial protocol. This is not directly supported from Arduino. But I found a working 9 bit hardware serial implementation. For convienience, I have added the working source code here because there a different versions available (in github and elsewhere). 

Caution: The 9 bit harware serial implementaion is depending on the correct version of the Ardunio IDE and the ProMicro board information. IDE 1.6.13  and board version 1.6.15 are working. Later versions changed the hardware serial implementation and the 9 bit changes will not work.

The three files from "NewHWSworking" have to be copied into the Arduino directory: "C:\Program Files (x86)\Arduino\hardware\arduino\avr\cores\arduino".

As Arduino board for the ProMicro you can either use the standard Arduino "Leonardo" board information or the ProMicro  SparkFun implementation: https://learn.sparkfun.com/tutorials/pro-micro--fio-v3-hookup-guide . Both will work.

The software is using several additional Arduino libraries (RCSwitch, Adafruit_GFX, Adafruit_SSD1306). These libraries have to be downloaded and installed. 

# Programming the 433 MHz Key Fob

You should change the four codes in the software to individual random numbers. The same codes have to be copied to the "_433_Programming.ino". Only for programming you will also need a 433 MHZ sender connected to the ProMicro.
Send each code individually and programm the keys of the remote according to the programming description of your KeyFob).

Key 1 = button A = -1 degree
Key 2 = button B =  +0 degree
Key 3 = button C = -10 degeees
Key 4 = button D = +10 degrees

Have fun with the remote control..

Plese be aware that this repository is only for educational purpose to learn how to program the seatalk protocol. Use it on your own risk and do not use it for critical systems in real life environments.
