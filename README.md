# Eurorack clock and control voltage source for Arduino

Software for controlling an Arduino based trigger and control voltage sequencer eurorack module.

Implementation created for personal use only. Not intented to be a collaborative project.

## What is this all about?

Eurorack is a modular synthesizer format. An eurorack synthesizer consists of multiple independent modules which send, receive, modulate, transform, etc. voltages. Some modules are sound generators, some are modulators, filters, voltage generators, mixers, envelope generators, CV sequencers and so on. Patching between modules are done with cables, and in the end of chain of modules we hopefully have a pleasant sythesized sound.

![Small eurorack system](https://github.com/arilaukkanen/eurorack-clock-and-cv-source/blob/main/images/eurorack.jpeg?raw=true)  
_A small eurorack modular system_

This code is implementation of a clock and control voltage source. Module can generate rhythmic triggers and continuous voltages. Module has eight channels, all can send triggers and the last four also control voltages. Tempo, swing, gate timing/length and all rhythmic and voltage patterns are user customizable.

User interface consists of 1.3" character display, a start/stop switch with 3.5mm CV input, a push-button rotary encoder for using menus, 8 LEDs for indicating state of each output and 8 3.5mm jacks for channel output voltages.

Code in this repo is meant to be flashed to Arduino microcontroller board. Certain arduino pins should be wired to and from 3.5mm audio jacks, and some pins to display module. Arduino power management should be handled with eurorack power rails. Jacks and display should be mounted on eurorack compatible front plate, which then can be installed to eurorack system. Not going into more details here.

## How to run

Clone the repo, get Arduino IDE, Arduino Micro, and all required HW components. Install required libraries, build and flash software to Arduino, do the wiring and build the HW, install to your eurorack system and enjoy!

![ClockWork module](https://github.com/arilaukkanen/eurorack-clock-and-cv-source/blob/main/images/clock-and-cv-source.jpeg?raw=true)  
_DIY ClockWork module based on the code_
