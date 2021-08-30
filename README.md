# DISCLAIMER 
* This project should be considered BETA. You decide to make and fit this. I will not take responsibility for any damage, however caused.
# OpenSwitcher
Amiga Microprocessor-controlled drive switcher with DF1 internal drive support. Optional SMD small version.

![openswitcher_pcb](https://user-images.githubusercontent.com/89555920/131311874-8140f8ab-ed11-46ad-838c-ce0a1a964648.png)
![openswitcher_pcb_smd_211s](https://user-images.githubusercontent.com/89555920/131362737-b4111971-3445-4ebe-80f9-1cbb9642c00e.png)




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
* JP1 is manual override. Default jumper - unconnected or 2-3. If 1-2 is linked, this sets permanent swap of DF0 and DF1, disabling long-press reset to switch modes.

# Optional bare minimum configuration
If you fit CIA socket and underside pins and nothing else, you can fit header pins to the middle pins of the relay and it will function like a traditional manual switch.

![openswitcher_pcb_simp_conf](https://user-images.githubusercontent.com/89555920/131317484-1bed9aa9-6bf9-468c-a090-e554a4068bd4.png)

# Parts List - Large Through-hole version
* 1 x 40 pin DIP dual wipe IC socket or female-male turned pins (2x20 pin strips)
* 2 x 20 pin male-male turned pin strips
* HFD3/5 5v non latching relay or Kemet EC2/EE2 relay or similar
* 4 x KSP2222 or similar NPN BJT transistor
* 3 x 1Kohm 1/4 watt resistors
* 1 x 150R 1/4 watt resistor
* 1 x 1n4148 or similar diode
* 1 x Adafruit Trinket M0 (recommended), though classic Trinket (3v and 5v are / will be supported)
* 1 x 2 pin header 2.54mm pitch
* 1 x DuPont cable female-soldered end (or female to male, to insert into keyboard connector pin 3

# Component Locations - Large Through-hole version
* U1 - Dual-wipe 40 pin IC socket or 2x20 female turned pin
* U2 - Male - male turned pin
* RL1 - HFD3/5 or other chosen relay
* D1 - 1N4148 diode
* R1 - 150R resistor
* R2, R3, R4 - 1K resistor
* Q1, Q2, Q3, Q4 - KSP2222 npn BJT transistor(600mA, 40v)
* P1 - Adafruit Trinket M0
* J1 - 1 x 2 header pin
* JP1 - 1 x 3 header pin (optional)

# Parts List - Slim SMD version
* 1 x 40 pin DIP dual wipe IC socket or female-male turned pins (2x20 pin strips)
* 2 x 20 pin male-male turned pin strips
* IM03TS / G6K-2P-DC5 or other chosen relay or similar
* 4 x BC817-40 npn BJT transistor
* 3 x 1Kohm 1206 0.25w resistor
* 1 x 150R 1206 0.25w resistor
* 1 x 1n4148 or similar diode
* 1 x Adafruit Trinket M0 (recommended), though classic Trinket (3v and 5v are / will be supported)
* 1 x 2 pin header 2.54mm pitch
* 1 x DuPont cable female-soldered end (or female to male, to insert into keyboard connector pin 3

# Component Locations - Slim SMD version
* U1 - Dual-wipe 40 pin IC socket or 2x20 female turned pin
* U2 - Male - male turned pin
* K1 - IM03TS / G6K-2P-DC5 or other chosen relay
* D1 - 1N4148 diode
* R1 - 150R 1206 0.25w resistor
* R2, R3, R4 - 1K 1206 0.25w resistor
* Q1, Q2, Q3, Q4 - BC817-40 npn BJT transistor(500mA, 45v)
* P1 - Adafruit Trinket M0
* J1 - 1 x 2 header pin
* JP1 - 1 x 3 header pin (optional)

# User Guide
* Remove even CIA from motherboard and fit into OpenSwitcher, ensuring correct orientation. Fit OpenSwitcher back into motherboard, ensuring pins are sitting in CIA socket.
* Fit wire from J1 pin 2 (nearest to Trinket) to CN13 pin 3 (keyboard header). It should be piggybacked onto the connection. Don't cut any wires! Or if you do cut, make sure you join them together again
* Ensure Trinket is NOT PLUGGED IN to OpenSwitcher
* Download software and plug in Trinket. A drive will open
* Drag and drop downloaded UF2 file to this drive. Wait for lights to go out on device then disconnect.
* Fit Trinket onto OpenSwitcher as the image on the OpenSwitcher board
* Complete

# Usage
* To swap internal drive to DF1 and external to DF0, hold down [CTRL] [AMIGA] [AMIGA] for 3 seconds until white LED shows then let go of keys.
* To swap internal drive to DF0 and external to DF1 (default, stock), hold down [CTRL] [AMIGA] [AMIGA] for 3 seconds until yellow LED shows then let go of keys.
* After 3 seconds, mode will be active.
