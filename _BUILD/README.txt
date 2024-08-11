A2DVI Firmware
==============
* A2DVI_vX.Y.uf2:
  Firmware for boards with the original PICO module (RP2040).

* A2DVI_vX.Y_PICO2.uf2:
  Firmware for boards with the newer PICO2 module (RP2350).


Firmware Update
===============
To update the firmware of the A2DVI card:

1. Switch off your Apple II. It's otherwise safe to update the A2DVI card while it is still plugged in the Apple II.
2. Press the white "BOOTSEL" button on top of the A2DVI's PICO module. Keep the button depressed.
3. While the button is still depressed, connect the USB cable between your PC/Mac and the PICO module.
4. Your PC/Mac should recognize a new mass storage drive.
5. Drag & drop the appropriate firmware file ("A2DVI_vX.Y.uf2" or "A2DVI_vX.Y_PICO2.uf2") to the drive.
6. After 2 seconds the LED should light up. The firmware update is complete.



Configuration Utility
=====================
There are two disk images with the configuration utility matching the firmware:

* A2DVICONFIG_DOS33.dsk:
  DOS 3.3 disk image. Suitable for Apple II, II+, //e.

* A2DVICONFIG_PRODOS.po:
  ProDOS disk image. Suitable for Apple //e.

Functionality of the two disks is identical. Just the DOS/ProDOS environment differs.



License & More Info
===================
The A2DVI firmware is released under the MIT License:
    https://github.com/ThorstenBr/A2DVI-Firmware
    Copyright (c) 2024 Thorsten Brehm

A2DVI was an idea by Ralle Palaveev. Hardware project:
    https://github.com/rallepalaveev/A2DVI
    Copyright (c) 2024 Ralle Palaveev
