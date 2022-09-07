# arduino-chicken-door

## Version 3
This version has most of the features removed and is focused on low power
consumption because I'm powering it off of a 12v battery.  Hardware:
- 3.3v Pro Mini Atmega 328P
- An L298N motor driver
- A 12" linear actuator
- A 12v -> 5v buck converter

This is all wired into a waterproof box.  I had this running on an old 12v
battery for several days and saw no decline in voltage.  However, I'm adding a
small solar panel to charge so I won't know how long it would last. If needed
there are [guides to lowering power
consumption](https://www.the-diy-life.com/making-an-ultra-low-power-arduino-pro/)
beyond just using the software library I'm using here.

## Version 2
This has been running for a couple of years with no problems except an L298N
module burning out and needing replacing.  I don't remember much about the
hardware though.

## Version 1
An arduino program to operate a chicken door.  This one is using an Arduino
MEGA.

This was inspired by [Dave Nave's chicken
door](https://davenaves.com/blog/interests-projects/chickens/chicken-coop/arduino-chicken-door/).
