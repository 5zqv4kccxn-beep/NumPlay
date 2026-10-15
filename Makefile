NWLINK ?= npx --yes -- nwlink@1.0.0
ARM_CC ?= arm-none-eabi-gcc
ARM_STRIP ?= arm-none-eabi-strip
B := build
VERSION := $(shell cat VERSION)
EADK_CFLAGS := $(shell $(NWLINK) eadk-cflags-device)
ARM_CFLAGS := -std=gnu11 $(EADK_CFLAGS) -DNP_VERSION='"$(VERSION)"' -Os -Wall -Wextra -Wno-unused-parameter -Ilauncher/src
ARM_LINK := -nostartfiles --specs=nano.specs -Wl,--relocatable -Wl,--gc-sections -Wl,-e,main -Wl,-u,eadk_app_name -Wl,-u,eadk_app_icon
SOURCES := $(wildcard launcher/src/*.c)
HEADERS := $(wildcard launcher/src/*.h) launcher/src/reader_docs.h VERSION

.PHONY: Tableur
Tableur: $(B)/Tableur.nwa

$(B):
	mkdir -p $(B)/arm $(B)/variant

$(B)/arm/%.o: launcher/src/%.c $(HEADERS) | $(B)
	$(ARM_CC) $(ARM_CFLAGS) -flto -c $< -o $@

$(B)/variant/Tableur-name.o: launcher/app_name.c Makefile | $(B)
	$(ARM_CC) $(ARM_CFLAGS) '-DNP_APP_NAME="Tableur"' -c $< -o $@

$(B)/variant/Tableur-icon.o: launcher/assets/icon.png | $(B)
	$(NWLINK) png-icon-o $< $@

$(B)/Tableur.nwa: $(patsubst launcher/src/%.c,$(B)/arm/%.o,$(SOURCES)) $(B)/variant/Tableur-name.o $(B)/variant/Tableur-icon.o
	$(ARM_CC) $(ARM_CFLAGS) $(ARM_LINK) -flinker-output=nolto-rel -o $@ $^ -lm -lgcc
	$(ARM_STRIP) --strip-unneeded $@
