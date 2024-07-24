RELEASE := RELEASE
FW_FILE := $(wildcard $(RELEASE)/A2DVI_v*.uf2)
DUMMY := $(warning FWFILE=$(FW_FILE))
VERSION := $(subst .uf2,,$(subst $(RELEASE)/A2DVI_,,$(FW_FILE)))

ZIP := A2DVI_$(VERSION).zip

all:
	rm -f $(RELEASE)/A2DVI_v*.*
	make -C $(RELEASE)
	make -C TEST
	make -C. pack

pack:
	echo "Packing $(ZIP)"
	cp README.txt $(RELEASE)/.
	cp ../configutil/A2DVICONFIG_PRODOS.po $(RELEASE)/.
	cp ../configutil/A2DVICONFIG_DOS33.dsk $(RELEASE)/.
	cd $(RELEASE) && zip $(ZIP) README.txt A2DVI_v*.uf2 A2DVICONFIG_*
	mv $(RELEASE)/$(ZIP) $(ZIP)

