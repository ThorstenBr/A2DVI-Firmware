AC = java -jar ../tools/AppleCommander-ac-1.9.0.jar

PRODOS_TEMPLATE = templates/ProDOS.po
DOS33_TEMPLATE  = templates/DOS3.3_MASTER.dsk

FONT1 = $(addprefix ../firmware/fonts/raw/,IIe_FR.bin)
FONT2 = $(addprefix ../firmware/fonts/raw/,IIe_US-DE.bin)
FONT3 = $(addprefix ../firmware/fonts/raw/,IIe_US.bin)
FONT4 = $(addprefix ../firmware/fonts/raw/,II_US.bin)
FONTS = $(FONT1) $(FONT2) $(FONT3) $(FONT4)

CONFIG_BAS = A2DVICONFIG.BAS

all: A2DVICONFIG_PRODOS.po A2DVICONFIG_DOS33.dsk

A2DVICONFIG_PRODOS.po: $(CONFIG_BAS) $(PRODOS_TEMPLATE) $(FONTS) Makefile
	cp $(PRODOS_TEMPLATE) _$@
	$(AC) -d _$@ STARTUP
	cat $< | $(AC) -bas _$@ STARTUP
	cat $(FONT1) | $(AC) -p _$@ IIE_FRENCH bin
	cat $(FONT2) | $(AC) -p _$@ IIE_GERMAN bin
	cat $(FONT3) | $(AC) -p _$@ IIE_US     bin
	cat $(FONT4) | $(AC) -p _$@ II_US      bin
	mv _$@ $@
	$(AC) -l $@

A2DVICONFIG_DOS33.dsk: $(CONFIG_BAS) $(DOS33_TEMPLATE) $(FONTS) Makefile
	cp $(DOS33_TEMPLATE) _$@
	$(AC) -d _$@ HELLO
	cat $< | $(AC) -bas _$@ HELLO
	cat $(FONT1) | $(AC) -p _$@ IIE_FRENCH bin
	cat $(FONT2) | $(AC) -p _$@ IIE_GERMAN bin
	cat $(FONT3) | $(AC) -p _$@ IIE_US     bin
	cat $(FONT4) | $(AC) -p _$@ II_US      bin
	mv _$@ $@
	$(AC) -l $@
