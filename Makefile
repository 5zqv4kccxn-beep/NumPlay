# Tableur : lecteur de cours pour NumWorks (sans jeux).
NWLINK ?= npx --yes -- nwlink@1.0.0
ARM_CC = arm-none-eabi-gcc
ARM_STRIP = arm-none-eabi-strip
B = build
VERSION := $(shell cat VERSION)
EADK_CFLAGS := $(shell $(NWLINK) eadk-cflags-device)
ARM_CFLAGS = -std=gnu11 $(EADK_CFLAGS) -DNP_VERSION='"$(VERSION)"' -Os -Wall -Wextra -Wno-unused-parameter -Ilauncher/src
ARM_LINK = -nostartfiles --specs=nano.specs -Wl,--relocatable -Wl,--gc-sections -Wl,-e,main -Wl,-u,eadk_app_name -Wl,-u,eadk_app_icon -Wl,-u,eadk_api_level

LAUNCHER_SRC = $(wildcard launcher/src/*.c)
LAUNCHER_H = $(wildcard launcher/src/*.h) launcher/src/reader_docs.h
ARM_OBJS := $(patsubst launcher/src/%.c,$(B)/arm/%.o,$(LAUNCHER_SRC))

VARIANTS = Tableur
NAME_Tableur = Tableur
ICON_Tableur = launcher/assets/icon.png

.PHONY: all nwa clean check
all: nwa

nwa: $(foreach v,$(VARIANTS),$(B)/$(v).nwa)

$(B):
	mkdir -p $(B)/arm $(B)/variant

$(B)/arm/%.o: launcher/src/%.c $(LAUNCHER_H) VERSION | $(B)
	$(ARM_CC) $(ARM_CFLAGS) -flto -c $< -o $@

$(B)/variant/%-name.o: launcher/app_name.c Makefile | $(B)
	$(ARM_CC) $(ARM_CFLAGS) '-DNP_APP_NAME=$(NAME_$*)' -c $< -o $@

$(B)/variant/%-icon.o: $$(ICON_$$*) | $(B)
	$(NWLINK) png-icon-o $< $@

$(B)/empty.o: | $(B)
	printf '' | $(ARM_CC) $(ARM_CFLAGS) -x c -c -o $@ -

define VARIANT_template
$(B)/$(1).nwa: $(ARM_OBJS) $(B)/empty.o $(B)/variant/$(1)-name.o $(B)/variant/$(1)-icon.o
	$(ARM_CC) $(ARM_CFLAGS) $(ARM_LINK) -flinker-output=nolto-rel $$^ -lm -lgcc -o $$@
	$(ARM_STRIP) --strip-unneeded $$@
endef

$(foreach v,$(VARIANTS),$(eval $(call VARIANT_template,$(v))))

check: nwa
	@for f in $(B)/*.nwa; do echo "$$f $$(wc -c < $$f) bytes"; done

clean:
	rm -rf $(B)
