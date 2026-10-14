.RECIPEPREFIX = >
NWLINK ?= npx --yes -- nwlink@1.0.0
ARM_CC = arm-none-eabi-gcc
ARM_OBJCOPY = arm-none-eabi-objcopy
ARM_STRIP = arm-none-eabi-strip
PY ?= python3
B = build
VERSION := $(shell cat VERSION)
EADK_CFLAGS := $(shell $(NWLINK) eadk-cflags-device)
ARM_CFLAGS = -std=gnu11 $(EADK_CFLAGS) -DNP_VERSION='"$(VERSION)"' -Os -Wall -Wextra -Wno-unused-parameter -Ilauncher/src
ARM_LINK = -nostartfiles --specs=nano.specs -Wl,--relocatable -Wl,--gc-sections -Wl,-e,main -Wl,-u,eadk_app_name -Wl,-u,eadk_app_icon -Wl,-u,eadk_api_level
LAUNCHER_SRC = $(wildcard launcher/src/*.c)
LAUNCHER_H = $(wildcard launcher/src/*.h) launcher/src/reader_docs.h
ICON = launcher/assets/icon.png
ARM_OBJS = $(patsubst launcher/src/%.c,$(B)/arm/%.o,$(LAUNCHER_SRC))

.PHONY: all nwa check clean

all: nwa

nwa: $(B)/NumPlay.nwa

$(B):
> mkdir -p $(B)/arm

$(B)/arm/%.o: launcher/src/%.c $(LAUNCHER_H) VERSION | $(B)
> $(ARM_CC) $(ARM_CFLAGS) -flto -c $< -o $@

$(B)/icon.o: $(ICON) | $(B)
> $(NWLINK) png-icon-o $< $@

$(B)/name.o: launcher/app_name.c Makefile | $(B)
> $(ARM_CC) $(ARM_CFLAGS) '-DNP_APP_NAME="NumPlay"' -c $< -o $@

$(B)/empty.o: | $(B)
> printf '' | $(ARM_CC) $(ARM_CFLAGS) -x c -c -o $@ -

$(B)/NumPlay.nwa: $(ARM_OBJS) $(B)/empty.o $(B)/name.o $(B)/icon.o
> $(ARM_CC) $(ARM_CFLAGS) $(ARM_LINK) -flinker-output=nolto-rel $^ -lm -lgcc -o $@
> $(ARM_STRIP) --strip-unneeded $@

check: nwa
> @echo "$(B)/NumPlay.nwa $$(wc -c < $(B)/NumPlay.nwa) bytes"

clean:
> rm -rf $(B)
