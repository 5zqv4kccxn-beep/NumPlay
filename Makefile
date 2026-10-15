# Tableur : lecteur de cours pour NumWorks (sans jeux).
NWLINK ?= npx --yes -- nwlink@1.0.0
ARM_CC = arm-none-eabi-gcc
ARM_STRIP = arm-none-eabi-strip
B = build
VERSION := $(shell cat VERSION)
EADK_CFLAGS := $(shell $(NWLINK) eadk-cflags-device)
ARM_CFLAGS = -std=gnu11 $(EADK_CFLAGS) -DNP_VERSION='"$(VERSION)"' -Os -Wall -Wextra -Wno-unused-parameter -Ilauncher/src -flto
ARM_LINK = -nostartfiles --specs=nano.specs -Wl,--relocatable -Wl,--gc-sections -Wl,-e,main -Wl,-u,eadk_app_name -Wl,-u,eadk_app_icon -Wl,-u,eadk_api_level

# Liste FIXE des sources : les anciens fichiers (matrices, settings, live, inflate) sont ignorés.
SRC_NAMES = compat font_data gfx home icons main platform sheet sys ui
ARM_OBJ = $(addprefix $(B)/arm/,$(addsuffix .o,$(SRC_NAMES)))

.PHONY: nwa check clean
nwa: $(B)/Tableur.nwa

$(B)/arm/%.o: launcher/src/%.c
	mkdir -p $(B)/arm
    $(ARM_CC) $(ARM_CFLAGS) -c $< -o $@

$(B)/variant/Tableur-name.o:
    mkdir -p $(B)/variant
    printf '%s\n' 'const char eadk_app_name[] __attribute__((section(".rodata.eadk_app_name"))) = "Tableur";' > $(B)/variant/Tableur-name.c
    $(ARM_CC) $(ARM_CFLAGS) -c $(B)/variant/Tableur-name.c -o $@

$(B)/variant/Tableur-icon.o:
    mkdir -p $(B)/variant
    printf '%s\n' '__attribute__((section(".rodata.eadk_app_icon"))) const unsigned char eadk_app_icon[1] = {0};' > $(B)/variant/Tableur-icon.c
    $(ARM_CC) $(ARM_CFLAGS) -c $(B)/variant/Tableur-icon.c -o $@

$(B)/Tableur.nwa: $(ARM_OBJ) $(B)/variant/Tableur-name.o $(B)/variant/Tableur-icon.o
    $(ARM_CC) $(ARM_CFLAGS) $(ARM_LINK) $^ -o $(B)/Tableur.elf
    $(NWLINK) elf-to-nwa $(B)/Tableur.elf $@

check:
    @test -f $(B)/Tableur.nwa && echo "OK : $(B)/Tableur.nwa" || (echo "ERREUR : nwa absent" && exit 1)

clean:
    rm -rf $(B)
