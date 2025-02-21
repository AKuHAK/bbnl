#.SILENT:
GIT_VERSION := $(shell git describe --always --dirty --tags --exclude nightly)

ELF_BASE_NAME := bbnl-$(GIT_VERSION)

EE_BIN = $(ELF_BASE_NAME)_unc.elf
EE_BIN_PKD = $(ELF_BASE_NAME).elf

EE_OBJS = main.o common.o config.o module_init.o game_id.o loader.o
EE_OBJS += neutrino.o opl.o pops.o
# Basic modules
IRX_FILES += iomanX.irx fileXio.irx
# BDM
IRX_FILES += ps2dev9.irx bdm.irx bdmfs_fatfs.irx ata_bd.irx
# Embedded ELF files
ELF_FILES += loader.elf

EE_LIBS = -lpatches -lgskit -ldmakit
EE_CFLAGS += -mno-gpopt -G0 -DGIT_VERSION="\"${GIT_VERSION}\""

EE_OBJS_DIR = obj/
EE_ASM_DIR = asm/
EE_SRC_DIR = src/

EE_OBJS += $(IRX_FILES:.irx=_irx.o)
EE_OBJS += $(ELF_FILES:.elf=_elf.o)
EE_OBJS := $(EE_OBJS:%=$(EE_OBJS_DIR)%)

EE_INCS := -Iinclude -I$(PS2DEV)/gsKit/include

EE_LDFLAGS := -L$(PS2DEV)/gsKit/lib -s

BIN2C = $(PS2SDK)/bin/bin2c

.PHONY: all clean

all: $(EE_BIN_PKD)

$(EE_BIN_PKD): $(EE_BIN)
	ps2-packer $< $@

clean:
	$(MAKE) -C loader clean
	rm -rf $(EE_BIN) $(EE_BIN_PKD) $(EE_ASM_DIR) $(EE_OBJS_DIR)

# ELF loader
loader/loader.elf: loader
	$(MAKE) -C $<

%loader_elf.c: loader/loader.elf
	$(BIN2C) $(*:$(EE_SRC_DIR)%=loader/%)loader.elf $@ $(*:$(EE_SRC_DIR)%=%)loader_elf

# IRX files
%_irx.c:
	$(BIN2C) $(PS2SDK)/iop/irx/$(*:$(EE_SRC_DIR)%=%).irx $@ $(*:$(EE_SRC_DIR)%=%)_irx

$(EE_ASM_DIR):
	@mkdir -p $@

$(EE_OBJS_DIR):
	@mkdir -p $@

$(EE_OBJS_DIR)%.o: $(EE_SRC_DIR)%.c | $(EE_OBJS_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(EE_OBJS_DIR)%.o: $(EE_ASM_DIR)%.c | $(EE_OBJS_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
