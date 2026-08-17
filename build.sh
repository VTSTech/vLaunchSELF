#!/bin/bash

# vLaunchSELF Build Script
# Based on the working build pattern from development

echo "========================================="
echo "vLaunchSELF Build Script"
echo "========================================="

# Create build directory if it doesn't exist
make clean
mkdir -p build

echo "Step 1: Compiling source files..."
make

if [ $? -ne 0 ]; then
    echo "❌ Compilation failed!"
    exit 1
fi

echo "✅ Compilation successful"

echo "Step 2: Stripping and creating SELF file..."
cp *elf build
ppu-strip build/vLaunchSELF.elf -o build/vLaunchSELF_stripped.elf
sprxlinker build/vLaunchSELF_stripped.elf
make_self build/vLaunchSELF_stripped.elf vLaunchSELF.self

if [ $? -ne 0 ]; then
    echo "❌ SELF creation failed!"
    exit 1
fi

echo "Step 3: Building PKG..."
make pkg

# Cleanup temporary files
rm -f vLaunchSELF.fake.self

echo "========================================="
echo "✅ Build Complete!"
echo "========================================="
echo "Output files:"
echo "  - build/vLaunchSELF.elf (executable)"
echo "  - build/vLaunchSELF_stripped.elf (stripped)"
echo "  - build/vLaunchSELF.pkg"
echo "  - vLaunchSELF.self (ready for testing)"
echo ""
echo "🚀 Ready to test in RPCS3!"
echo "========================================="
