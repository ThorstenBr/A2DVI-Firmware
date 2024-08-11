PICO_RELEASE  := PICO/RELEASE

FW_FILE := $(wildcard $(PICO_RELEASE)/A2DVI_v*.uf2)
DUMMY := $(warning FWFILE=$(FW_FILE))
VERSION := $(subst .uf2,,$(subst $(PICO_RELEASE)/A2DVI_,,$(FW_FILE)))

ZIP := A2DVI_$(VERSION).zip

.PHONY: all prepare pico pico2 pack

all: prepare pico
	make -C. pack

all_pico2: prepare pico pico2
	make -C. pack

prepare:
	mkdir -p RELEASE
	rm -f RELEASE/A2DVI_v*.* RELEASE/A2DVICONFIG_*

pico:
	rm -f PICO/RELEASE/A2DVI_v*.*
	make -C PICO/RELEASE
	make -C PICO/TEST
	cp PICO/RELEASE/A2DVI_v*.uf2 RELEASE/.

pico2:
	rm -f PICO2/RELEASE/A2DVI_v*.*
	make -C PICO2/RELEASE
	make -C PICO2/TEST
	cp PICO2/RELEASE/A2DVI_v*_PICO2.uf2 RELEASE/.

pack:
	echo "Packing $(ZIP)"
	cp README.txt RELEASE/.
	cp ../configutil/A2DVICONFIG_PRODOS.po RELEASE/.
	cp ../configutil/A2DVICONFIG_DOS33.dsk RELEASE/.
	cd RELEASE && zip $(ZIP) README.txt A2DVI_$(VERSION)*.uf2 A2DVICONFIG_*
	mv RELEASE/$(ZIP) $(ZIP)
