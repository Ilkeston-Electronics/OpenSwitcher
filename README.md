# OpenSwitcher
Amiga Microprocessor-controlled drive switcher with DF1 internal drive support. Optional SMD small version.

# Latest firmware in releases.

![OpenSwitcher_Final](https://user-images.githubusercontent.com/89555920/132751234-ff0f4966-17bc-42ac-8236-0587b59d51c2.png)



# Description

There are lots of (very simple) Amiga drive switcher projects out there. This one intends to be a little different. Driven by logic and very simple code, designed to be user buildable and programmable, with no fancy programmers required. If you have USB, you can program this with a single file! Tested with A500 plus and kickstarts v2.04, v2.05, v3.1 and v3.1.4. Compatible with Pistorm (though pistorm has its own swapping system).

# Project Aims

To build a quality Amiga drive swapper out of "off the shelf" parts, easily available and easily programmable. Also have the internal drive functioning as DF1 via correct drive ID timing during switching.

# But why?
Our Amigas deserve the best. All other drive swapping projects I have seen have the following caveats... Drilling holes, fitting switches, programming PIC chips, fitting surface-mount components, internal drive not working... This project aims to address all those.
Amigas are very fussy about which drive we run from. DF1 is a bootable drive, but it isn't DF0. Which is sadly hard-coded into the software, leading to problems.

# Working Principle
How does this work?
OK... There are 2 lines we need to swap, which are SEL0 and SEL1. Simple enough...

This project uses an Adafruit Trinket M0 microcontroller.

This uses a relay so any signal is "pure", ie it is just like swapping the SEL points with solder or a switch. The SEL signal is passed through the relay just as it left the CIA.
This switching is controlled by the Trinket, which has levels corrected by the transistors, which then goes to the relays.

* Q2 and Q3 are sel0 / sel1 sensor lines. Only sel0 sensor line is used at the time of writing.
* Q1 base is connected to relay coil Upon a base positive voltage, coil - GND goes low resistance, switching on the relay.
* J1 is J_RST_LINE. Pin 1 default not connected. Pin 2 monitors keyboard reset line. If high, returns low to microcontroller. If using / testing without keyboard reset line connected, jumper to pin 1 to stop automatic rebooting and eeprom writing of microcontroller.
* JP1 is used as part of a latching relay system. Link points marked on board if using a latching relay

# Optional bare minimum configuration
If you fit CIA socket and underside pins and nothing else, you can fit header pins to the middle pins of the relay and it will function like a traditional manual switch.

![openswitcher_pcb_simp_conf](https://user-images.githubusercontent.com/89555920/131317484-1bed9aa9-6bf9-468c-a090-e554a4068bd4.png)

# Parts List - Large Through-hole version
* 1 x 40 pin DIP dual wipe IC socket or female-male turned pins (2x20 pin strips)
* 2 x 20 pin male-male turned pin strips
* Relay (see below)
* 4 (or 5) x KSP2222 or similar NPN BJT transistor
* 3 x 1Kohm 1/4 watt resistors
* 1 (or 2) x 470R 1/4 watt resistor
* 1 (or 2) x 1n4148 or similar diode
* 1 x Adafruit Trinket M0 (recommended), though classic Trinket (3v and 5v are / will be supported)
* 1 x 2 pin header 2.54mm pitch
* 1 x DuPont cable female-soldered end (or female to male, to insert into keyboard connector pin 3
* 2 x 5 male pin header (2.54mm pitch supplied with Trinket)
* 2 x 5 female pin header (2.54mm pitch to plug Trinket in to, or solder Trinket into board)

# Component Locations - Large Through-hole version
* U1 - Dual-wipe 40 pin IC socket or 2x20 female turned pin
* U2 - Male - male turned pin
* RL1 - Relay (see below)
* D1, D2 - 1N4148 diode
* R1, R5 - 470R resistor
* R2, R3, R4 - 1K resistor
* Q1, Q2, Q3, Q4, Q5 - KSP2222 npn BJT transistor(600mA, 40v)
* P1 - Adafruit Trinket M0
* J1 - 1 x 2 header pin
* JP1 - 1 x 3 header pin (optional)

# Parts List - Slim SMD version
* 1 x 40 pin DIP dual wipe IC socket or female-male turned pins (2x20 pin strips)
* 2 x 20 pin male-male turned pin strips
* Relay (see below)
* 4 (or 5) x BC817-40 npn BJT transistor
* 3 x 1Kohm 0402 0.06w resistor
* 1 (or 2) x 0402 0.06w 0.25w resistor
* 2 x 1n4148 or similar diode
* 1 x Adafruit Trinket M0 (recommended), though classic Trinket (3v and 5v are / will be supported)
* 1 x 2 pin header 2.54mm pitch
* 1 x DuPont cable female-soldered end (or female to male, to insert into keyboard connector pin 3
* 2 x 5 male pin header (2.54mm pitch supplied with Trinket)
* 2 x 5 female pin header (2.54mm pitch to plug Trinket in to, or solder Trinket into board)

# Component Locations - Slim SMD version
* U1 - Dual-wipe 40 pin IC socket or 2x20 female turned pin
* U2 - Male - male turned pin
* K1 - Relay (see below)
* D1, D2 - 1N4148 diode
* R1, R5 - 470R 0402 0.06w resistor
* R2, R3, R4 - 1K 0402 0.06w resistor
* Q1, Q2, Q3, Q4, Q5 - BC817-40 npn BJT transistor(500mA, 45v)
* P1 - Adafruit Trinket M0
* J1 - 1 x 2 header pin
* JP1 - 1 x 3 header pin (optional)

# Supported relays
* Any 5v relay that will fit into the footprint
# Latching relays (need D2, R5 and Q5 fitting)
* Panasonic TX2-L-5V / TX2-L2-5V
* Hongfa HFD3/005-L2
* Kemet EE2-5T
# Non-Latching relays
* TE Connectivity IM03TS
* TE Connectivity 1-1462038-3
* Omron G6K2P5DC
* Omron G6K-2P-Y 5DC
* Hongfa HFD4/005

# Notes about relays
* You can either use a latching or non-latching relay. Please pay attention to the following points.
* If using a non-latching relay, no need to fit D2, R5 or Q5. JP1 should remain open and unlinked.
* If using latching relay, D2, R5 and Q5 is required to set / reset relay. JP1 should be linked where indicated on the board.

# User Guide
* Remove even CIA from motherboard and fit into OpenSwitcher, ensuring correct orientation. Fit OpenSwitcher back into motherboard, ensuring pins are sitting in CIA socket.
* Fit wire from J1 pin 2 (marked on board) to CN13 pin 3 (keyboard header). It should be piggybacked onto the connection. Don't cut any wires! Or if you do cut, make sure you join them together again
* Ensure Trinket is NOT PLUGGED IN to OpenSwitcher or CIA is removed from OpenSwitcher (I haven't had any failed CIA chips during flashing, but better to be safe)
* Download software and plug in Trinket. A drive will open
* Drag and drop downloaded UF2 file to this drive. Wait for lights to go out on device then disconnect.
* Fit Trinket onto OpenSwitcher as the image on the OpenSwitcher board
* Complete

# Usage
* Operating Guide

* >>>>>>START<<<<<<
* Upon power on, unconnected (ie plugged into PC), red light will be flashing. This means successfully flashed... Please plug into OpenSwitcher.
* ------------------------------
* This will ONLY HAPPEN ONCE on FIRST INSTALL INTO OpenSwitcher! Upon Amiga power-on, white LED should flash 3 times. Identify relay, program virtual Eeprom with default values. When white LED flash 3 times This indicates setup is correct.
* ------------------------------
* Upon Green LED, drive ID successful, booting in a stock unswapped condition
* Upon Blue LED, drive ID successful, booting in swapped condition.
* LED off after 8 seconds approx = OpenSwitcher is asleep.

* Upon Amiga RESET, pink LED, reboot successful, go back to start.
* If Amiga RESET is HELD for approx 3 seconds, White LED will blink 3 times. If in a swapped condition, will become unswapped. If unswapped, will be swapped. Stored to Eeprom.
* That is all
# DISCLAIMER 
* This project should be considered BETA. You decide to make and fit this. I will not take responsibility for any damage, however caused.

