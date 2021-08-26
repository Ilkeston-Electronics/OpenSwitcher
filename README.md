# OpenSwitcher
Amiga Microprocessor-controlled drive switcher

Description
There are lots of (very simple) Amiga drive switcher projects out there. This one intends to be a little different. Driven by logic and very simple code, designed to be user buildable and programmable, with no fancy programmers required. If you have USB, you can program this with a single file!

Project Aims
To build a quality Amiga drive swapper out of "off the shelf" parts, easily available and easily programmable.

But why?
Our Amigas deserve the best. All other drive swapping projects I have seen have the following caveats... Drilling holes, fitting switches, programming PIC chips, fitting surface-mount components, internal drive not working... This project aims to address all those.
Amigas are very fussy about which drive we run from. DF1 is a bootable drive, but it isn't DF0. Which is sadly hard-coded into the software, leading to problems.

How does this work?
OK... There are 2 lines we need to swap, which are SEL0 and SEL1. Simple enough...

This project uses an Adafruit Trinket (5v, 3.3v or M0 is fine - all will be supported), 3 transistors and a latching relay.

This uses a latching relay so any signal is "pure", ie it is just like swapping the SEL points with solder or a switch. The SEL signal is passed through the relay just as it left the CIA.
This switching is controlled by the Trinket, which has levels corrected by the transistors, which then goes to the relays.
...continued
