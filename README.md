# Eurorack clock and control voltage source for Arduino

Implementation for personal use only. Not intented to be shared in any way.

## What is this all about?

Eurorack is a modular synthesizer format. An eurorack synthesizer consists of multiple independent modules which send, receive, modulate, transform, etc. voltages. Some modules are sound generators, some are modulators, filters, voltage generators and so on. Patching between modules are done with cables. And in the end of chain, we hopefully have a pleasant sythesized sound.

Code in this repo is meant to be flashed to Arduino microcontroller board. Certain arduino pins should be wired to and from 3.5mm audio jacks, and some pins to display module. Arduino power management should be handled with eurorack power rails. Jacks and display should be mounted on eurorack compatible front plate, which then can be installed to eurorack system. Not going into more details here.

## How to run

Clone the repo, get Arduino IDE, Arduino Micro, and all required HW components. Flash software to Arduino, do the wiring and build the HW, install to your eurorack system and enjoy!
