# vLaunchSELF - ELF/SELF Launcher
#  
# A PS3 file browser with ELF/SELF launching capability.
# Based on PS3-Moonlight graphics and text rendering system
# Original: https://github.com/Cruslan/PS3-Moonlight

TARGET := vLaunchSELF
TARGET_ELF := $(TARGET).elf
TARGET_STRIPPED := $(TARGET)-stripped.elf
APPID := VTSTECH02
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
stripped: $(TARGET_STRIPPED)

$(TARGET_ELF): $(OFILES)
	$(CC) $(OFILES) $(LDFLAGS) $(LIBS) -o $@

$(TARGET_STRIPPED): $(TARGET_ELF)
	$(PS3DEV)/ppu/bin/ppu-strip $< -o $@

-include $(OFILES:.o=.d)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Create a SELF file from ELF (exact same process as PKG EBOOT.BIN)
self: elf
	@mkdir -p build
	@$(PS3DEV)/ppu/bin/ppu-strip $(TARGET_ELF) -o ./build/$(TARGET_STRIPPED)
	@echo "Creating SELF with exact same command as PKG..."
	@$(PS3DEV)/bin/make_self_npdrm ./build/$(TARGET_STRIPPED) $(TARGET).self $(CONTENTID) 2>/dev/null || $(PS3DEV)/bin/make_self ./build/$(TARGET_STRIPPED) $(TARGET).self
	@echo "Checking if SELF file is valid..."
	@hexdump -C $(TARGET).self | head -5 || echo "hexdump failed"
	@ls -la $(TARGET).self
	@echo "Comparing with PKG EBOOT.BIN..."
	@ls -la build/pkg/USRDIR/EBOOT.BIN
	@diff $(TARGET).self build/pkg/USRDIR/EBOOT.BIN > /dev/null 2>&1 && echo "✓ Files are identical" || echo "⚠ Files are different"

# Create a PKG package for PS3 (uses stripped ELF for RPCS3 compatibility)
pkg: stripped
	@mkdir -p build/pkg/USRDIR
	@echo "Copying stripped ELF to EBOOT.BIN..."
	@cp $(TARGET_STRIPPED) build/pkg/USRDIR/EBOOT.BIN
	@echo "Before signing:"
	@ls -la build/pkg/USRDIR/EBOOT.BIN
	@$(PS3DEV)/bin/sprxlinker build/pkg/USRDIR/EBOOT.BIN 2>/dev/null || true
	@$(PS3DEV)/bin/sfo -f sfo.xml build/pkg/PARAM.SFO
	@# Sign EBOOT.BIN with NPDRM for real PS3 compatibility
	@echo "Applying NPDRM signing..."
	@$(PS3DEV)/bin/make_self_npdrm build/pkg/USRDIR/EBOOT.BIN build/pkg/USRDIR/EBOOT.BIN $(CONTENTID) 2>/dev/null || true
	@echo "After signing:"
	@ls -la build/pkg/USRDIR/EBOOT.BIN
	@$(PS3DEV)/bin/pkg --contentid $(CONTENTID) build/pkg/ build/$(TARGET).pkg
	@echo "Built PKG: build/$(TARGET).pkg"

# Create both SELF and PKG
all_pkg: pkg self

# Clean build artifacts
clean:
	rm -f $(OFILES) $(OFILES:.o=.d) $(TARGET_ELF) $(TARGET_STRIPPED) $(TARGET).self
	rm -rf build
