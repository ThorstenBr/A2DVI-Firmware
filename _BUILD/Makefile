# MIT License
# Copyright (c) 2024 Thorsten Brehm
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

PICO_RELEASE  := PICO/RELEASE

# determine current firmware version
FW_FILE := $(wildcard $(PICO_RELEASE)/A2DVI_v*.uf2)
ifneq ($(FW_FILE),)
# version only available in recursive make stages, when firmware is already built
VERSION := $(subst .uf2,,$(subst $(PICO_RELEASE)/A2DVI_,,$(FW_FILE)))
endif

# optional include for local path settings (PICO_SDK_PATH, PICO_TOOLCHAIN_PATH)
-include localsettings.mk

export PICO_TOOLCHAIN_PATH
export PICO_SDK_PATH

# result ZIP file
ZIP := A2DVI_$(VERSION).zip

.PHONY: all prepare pico pico2 pack

# build for PICO modules
all: prepare pico
	make -C. pack

# build for PICO2 modules
all_pico2: prepare pico pico2
	make -C. pack

# prepare release output directory
prepare:
	mkdir -p RELEASE
	rm -f RELEASE/A2DVI_v*.* RELEASE/A2DVICONFIG_*

# build for PICO modules
pico: PICO/RELEASE PICO/TEST
	rm -f PICO/RELEASE/A2DVI_v*.*
	make -C PICO/RELEASE
	make -C PICO/TEST
	cp PICO/RELEASE/A2DVI_v*.uf2 RELEASE/.

# build for PICO2 modules
pico2: PICO2/RELEASE PICO2/TEST
	rm -f PICO2/RELEASE/A2DVI_v*.*
	make -C PICO2/RELEASE
	make -C PICO2/TEST
	cp PICO2/RELEASE/A2DVI_v*_PICO2.uf2 RELEASE/.

# pack release files
pack:
	@echo "Packing $(ZIP)"
	cp README.txt RELEASE/.
	cp ../configutil/A2DVICONFIG_PRODOS.po RELEASE/.
	cp ../configutil/A2DVICONFIG_DOS33.dsk RELEASE/.
	cd RELEASE && zip $(ZIP) README.txt A2DVI_$(VERSION)*.uf2 A2DVICONFIG_*
	mv RELEASE/$(ZIP) $(ZIP)

# create directories and call cmake
PICO/RELEASE PICO/TEST PICO2/RELEASE PICO2/TEST:
	mkdir -p $@
	cd $@ && cmake ../../../firmware $(if $(findstring PICO2,$@),-DFEATURE_PICO2:bool=true) $(if $(findstring TEST,$@),-DFEATURE_TEST:bool=true)
	@echo "--------------------------------------------------------------"

# wipe directories for fresh builds
clean:
	rm -rf PICO/RELEASE PICO/TEST PICO2/RELEASE PICO2/TEST RELEASE
