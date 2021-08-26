# OpenSwitcher
Amiga Microprocessor-controlled drive switcher
![openswitcher_final](https://user-images.githubusercontent.com/89555920/130957064-b8d7032a-a47f-4047-8594-877e5de32a28.jpg)
![openswitcher_final_pop](https://user-images.githubusercontent.com/89555920/130957066-e871e3a5-0786-46ad-b889-ba67169dd9b5.jpg)


# Description

There are lots of (very simple) Amiga drive switcher projects out there. This one intends to be a little different. Driven by logic and very simple code, designed to be user buildable and programmable, with no fancy programmers required. If you have USB, you can program this with a single file!

# Project Aims

To build a quality Amiga drive swapper out of "off the shelf" parts, easily available and easily programmable. Also have the internal drive functioning as DF1 via correct drive ID timing during switching.

# But why?
Our Amigas deserve the best. All other drive swapping projects I have seen have the following caveats... Drilling holes, fitting switches, programming PIC chips, fitting surface-mount components, internal drive not working... This project aims to address all those.
Amigas are very fussy about which drive we run from. DF1 is a bootable drive, but it isn't DF0. Which is sadly hard-coded into the software, leading to problems.

# Working Principle
How does this work?
OK... There are 2 lines we need to swap, which are SEL0 and SEL1. Simple enough...

This project uses an Adafruit Trinket microcontroller (5v, 3.3v or M0 is fine - all will be supported), 3 transistors and a latching relay.

This uses a latching relay so any signal is "pure", ie it is just like swapping the SEL points with solder or a switch. The SEL signal is passed through the relay just as it left the CIA.
This switching is controlled by the Trinket, which has levels corrected by the transistors, which then goes to the relays.

Q4 and Q3 bases connect to microcontroller GPIO, which in turn switch SET and RESET on the relay accordingly.
Q1 base is connected to J1 pin 2 and this connects to the Amiga reset line from the keyboard (active high).
J1 pin 1 is an alternative reset input which is "active low", settable in the microcontroller software.
P2 pins 1 and 3 are manual active high connections, which can be supplied with 3v - 5v (possibly 12v) (5v recommended) that switch the relay manually. Pin 1 if supplied with voltage will activate Q3 base which resets the relay. Pin 2 will activate Q4 base which sets the relay. The centre pin is +5v for convenience.

# Optional bare minimum configuration
If you fit CIA socket and underside pins and nothing else, you can fit header pins to the middle pins of the relay and it will function like a traditional manual switch.

![bareminimum](https://user-images.githubusercontent.com/89555920/130951893-b6ab5bde-1879-4004-94aa-9b5d316510d3.jpg)

# Parts List
1 x 40 pin DIP dual wipe IC socket or female-male turned pins (2x20 pin strips)

2 x 20 pin male-male turned pin strips

V23079-B1201-B301 or HFD3/005-L2 (HFD3/005-L1 is NOT SUITABLE as it only has 1 coil) or similar 5v dual coil DPDT relay.

3 x KSP2222 or similar NPN BJT transistor

3 x 1Kohm 1/4 watt resistors

2 x 1n4148 or similar diodes

1 x Adafruit Trinket M0 (recommended), though classic Trinket (3v and 5v are / will be supported)

1 x 2 pin header 2.54mm pitch

1 x DuPont cable female-soldered end

1 x 250ohm - 1k ohm resistor for LED current limiting (optional)

2 x 2.54 pitch generic LED (optional)

1 x 3 pin header 2.54mm pitch (optional)






