# Installation

## Slot
You can plug the A2DVI card in any slot (1-7). The slot does not matter.

## Videx
If you want to use the Videx output (80 column support for Apple II) then your real Videx card must be installed in slot 3
(it provides the RAM, ROM and registers to your Apple II).
The A2DVI card is then installed in any other slot (1-2 or 4-7).

# Language or Custom Switch
You can connect the A2DVI to the language rocker switch of your Euro //e, so it correctly switches the character set.
You can also enable an advanced option, so the switch gets extra behavior: quickly rocking the switch back and forth cycles through the video modes (color/monochrome etc).

Being able to control the video mode without entering the menu is a convenient feature. If you have a machine without the switch (US machines), you may want to add a custom switch to your A2DVI.

## Jumpers and Connections
There are two solder jumpers on the bottom side of the A2DVI PCB. Make sure the "ALTCHR" jumper is closed. The "SYNC" jumper **must** remain open:

![Solder Jumpers](/images/A2DVI_SolderJumper.jpg)

There is a pin on the top side, labelled "ALTCHR". This is the input pin to connect the switch:

![ALTCHR input pin](/images/A2DVI_ALTCHR_pin.jpg)

## Euro //e
Apple installed a language rocker switch in the Euro //e.
The switch controlled the keyboard ROM (keyboard mapping) and the video ROM (character output).

In order for the language switch to work, an additional wire needs to be installed.

Connect the single pin on the A2DVI card to the language switch signal "ALTCHR", at J19, front pin facing towards the keyboard:

![Wiring the language switch](/images/A2DVI_ALTCHR_wire.jpg)

![Language Rocker Switch](/images/ALTCHR_Rocker_Switch.jpg)

## Installing a Custom Switch
If your machine does not have a language switch, you can manually install an external switch.

This is how the switch should be connected:

![Switch Connection](/images/A2DVI_SwitchConnection.jpg)

The switch connects to +5V, *not* to ground.

The 1KOhm resistor is optional. Apple's orignal Euro //e did not have this.
But you may want to use a resistor for protection (to prevent a short with the full current of your power-supply, if the switch ever got in contact with something else).
