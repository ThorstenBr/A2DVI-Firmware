# A2DVI Firmware: Apple II Digital Video

This is a firmware project for a digital DVI/HDMI Apple II video card.

The project is currently being tested. Firmware builds and gerber files
will be published once initial testing is completed. Currently (July 2024)
we're waiting for a batch of updated PCBs.

Please be a little patient. Check out the discussion topic in the
AppleFritter forum, if you want to help with testing.

# How A2DVI Works

# Project Status

## Completed Features

* **40 column mode**: implemented.

* **80 column mode**: implemented.

* **LORES graphics**: implemented.

* **Double-LORES graphics**: implemented in monochrome only. Color support not implemented yet.

* **HIRES graphics**: implemented. Colors need to be verified/corrected.

* **Double-HIRES graphics**: implemented.

* **graphics/text mix modes**: implemented for all supported graphics modes.

* **Scanline emulation**: implemented and configurable.

* **Monochrome mode** and **monochrome color** configurable (b/w, green, amber).

* **Character set** configurable.

* **Language/rocker switch** suport for Euro-Apple IIs.

* **Flash** configuration.

* Apple II **Card registers** for configuration.

* **Configuration menu**

## TODOs

* Double-LORES in color.

* Load custom fonts.

# License
The A2DVI firmware is released under the [MIT License](/LICENSE):

* Copyright (c) 2024 Thorsten Brehm

<br>

A2DVI was an idea by Ralle Palaveev. His matching hardware project:

* [A2DVI Hardware](https://github.com/rallepalaveev/A2DVI). MIT License. Copyright (c) 2024 Ralle Palaveev.

<br><br>
The firmware project is based on awesome work from the following Apple II Analog VGA projects:

* [AppleII-VGA](https://github.com/markadev/AppleII-VGA). MIT License. Copyright (c) 2021-2022 Mark Aikens.

* [Analog Firmware](https://github.com/V2RetroComputing/analog-firmware). MIT License. Copyright (c) 2022-2023 David Kuder.

<br><br>
This project uses a customized variant of the PicoDVI library - and reuses many of Luke's genius ideas for the custom *Apple II to DVI/TMDS* encoding:

* [PicoDVI](https://github.com/Wren6991/PicoDVI). [BSD3 License](/libraries/libdvi/LICENSE). Copyright (c) 2021 Luke Wren.

<br><br>
*Apple II Forever!*
