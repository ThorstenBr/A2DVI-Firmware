# A2DVI Configuration

The firmware supports various configuration properties.

![Config Utility](/images/A2DVI_Config.jpg)

## Configuration Disks
There are configuration [utility disks for ProDOS](https://github.com/ThorstenBr/A2DVI-Firmware/raw/master/configutil/A2DVICONFIG_PRODOS.po) and [DOS3.3](https://github.com/ThorstenBr/A2DVI-Firmware/raw/master/configutil/A2DVICONFIG_DOS33.dsk).
The disk support the configuration of the card and uploading custom fonts (video ROMs).

Download the files and transfer them to a disk using ADTpro or similar.

## Configuration Program
Alternatively, you can type the program and save it to a disk manually.
It takes just a couple of lines to activate the menu,
since the menu is implemented inside the A2DVI firmware.

This is just how we would have done it in the 80s, when we manually copied and typed program listings from magazines... :)

The A2DVI configuration utility:

    10 HOME : PRINT "ENTER A2DVI SLOT (1-7): ";
    20 GET S
    30 IF (S<1) OR (S>7) THEN GOTO 20
    40 PRINT "LOOK AT YOUR A2DVI SCREEN!"
    50 VTAB 21
    60 S = -16256 + S*16
    70 POKE S+15,11
    80 POKE S+15,22
    90 POKE S+9,0
    100 GET A$
    110 C = ASC(A$)
    120 POKE S+9,C
    130 IF C <> 27 THEN GOTO 100
    140 POKE S+15,0
    150 HOME : PRINT "GOOD BYE!"

Uploading of custom video ROMs is not supported by this shortened version. The full utility is available [here](A2DVICONFIG.BAS).
