# vLaunchSELF - ELF/SELF Launcher
#  
# A PS3 file browser with ELF/SELF launching capability.
# Based on PS3-Moonlight graphics and text rendering system
# Original: https://github.com/Cruslan/PS3-Moonlight

TARGET := vLaunchSELF
TARGET_ELF := $(TARGET).elf
TARGET_STIPPED := $(TARGET)-stripped.elf
APPID := HLFB00001
CONTENTID := UP0001-$(APPID)_00-0000000000000000

# PS3SDK paths - assumes PSL1GHT is set up at /usr/local/ps3dev
-include $(PS3DEV)/ppu_rules

# Compiler settings
CC := $(PS3DEV)/ppu/bin/ppu-gcc
CFLAGS += -mcpu=cell -O2 -Wall -Wextra -Werror=implicit-function-declaration -MMD -MP
CFLAGS += -I$(PS3DEV)/ppu/include -I$(PS3DEV)/portlibs/ppu/include

# Libraries needed for graphics and text rendering
LIBS := -L$(PS3DEV)/ppu/lib \
         -L$(PS3DEV)/portlibs/ppu/lib \
         -ltiny3d -lfont3d -lgcm_sys -lrsx -lsysutil -lio -lsysmodule -lsysfs -lrt -lm

# Source file
OFILES := src/main.o

.PHONY: all elf stripped self pkg clean

all: elf stripped

# Build debug ELF
elf: $(TARGET_ELF)

# Build stripped ELF (for RPCS3 compatibility)
stripped: $(TARGET_STIPPED)

$(TARGET_ELF): $(OFILES)
	$(CC) $(OFILES) $(LDFLAGS) $(LIBS) -o $@

$(TARGET_STIPPED): $(TARGET_ELF)
	$(PS3DEV)/ppu/bin/ppu-strip $< -o $@

-include $(OFILES:.o=.d)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Create a SELF file from ELF (using make_self for proper signing)
self: elf
	@$(PS3DEV)/bin/ppu-strip $(TARGET).elf -o build/$(TARGET).elf.stripped
	@$(PS3DEV)/bin/make_self build/$(TARGET).elf.stripped $(TARGET).self
	@echo "Built SELF: $(TARGET).self"

# Create a PKG package for PS3 (uses stripped ELF for RPCS3 compatibility)
pkg: stripped
	@mkdir -p build/pkg/USRDIR
	@cp $(TARGET_STIPPED) build/pkg/USRDIR/EBOOT.BIN
	@$(PS3DEV)/bin/sprxlinker build/pkg/USRDIR/EBOOT.BIN 2>/dev/null || true
	@$(PS3DEV)/bin/sfo -f sfo.xml build/pkg/PARAM.SFO
	@# Sign EBOOT.BIN with NPDRM for real PS3 compatibility
	@$(PS3DEV)/bin/make_self_npdrm build/pkg/USRDIR/EBOOT.BIN build/pkg/USRDIR/EBOOT.BIN $(CONTENTID) 2>/dev/null || true
	@$(PS3DEV)/bin/pkg --contentid $(CONTENTID) build/pkg/ build/$(TARGET).pkg
	@echo "Built PKG: build/$(TARGET).pkg"

# Create both SELF and PKG
all_pkg: pkg self

# Clean build artifacts
clean:
	rm -f $(OFILES) $(OFILES:.o=.d) $(TARGET_ELF) $(TARGET_STIPPED) $(TARGET).self
	rm -rf build