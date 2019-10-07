# Engemu

An attempt to create an HLE N-Gage Emulator. The project is written C++ (despite github sometimews showing PHP code strange reasons).

## Status

The code is being cleaned and re-organized as a seperate entity. To look more into the code please have a look at :
https://github.com/mrRosset/Nomu

## CPU Emulation

Based on both my own Armv4T cpu emulator and [Unicorn](https://www.unicorn-engine.org/). Since Unicorn doesn't support any of the elevated kernel modes, work is being done to add that to my own emulator.

## Memory Emulation

Based on the details leaked thanks to the THC group:
**link removed**

![E32Explorer screenshot](https://i.imgur.com/TArbA3g.png)
