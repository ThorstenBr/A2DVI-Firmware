AC = java -jar ../tools/AppleCommander-ac-1.9.0.jar

PRODOS_TEMPLATE = templates/ProDOS.po
DOS33_TEMPLATE  = templates/DOS3.3_MASTER.dsk

CONFIG_BAS = A2DVICONFIG.BAS

all: A2DVICONFIG_PRODOS.po A2DVICONFIG_DOS33.dsk

A2DVICONFIG_PRODOS.po: $(CONFIG_BAS) $(PRODOS_TEMPLATE)
	cp $(PRODOS_TEMPLATE) _$@
	$(AC) -d _$@ STARTUP
	cat $< | $(AC) -bas _$@ STARTUP
	mv _$@ $@
	$(AC) -l $@

A2DVICONFIG_DOS33.dsk: $(CONFIG_BAS) $(DOS33_TEMPLATE)
	cp $(DOS33_TEMPLATE) _$@
	$(AC) -d _$@ HELLO
	cat $< | $(AC) -bas _$@ HELLO
	mv _$@ $@
	$(AC) -l $@
