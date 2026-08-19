#!/bin/bash

echo "=== Testing SELF Creation Process ==="

# Clean up any existing files
rm -f test_self_1 test_self_2 test_stripped.elf

# Create stripped ELF
echo "1. Creating stripped ELF..."
/usr/local/ps3dev/ppu/bin/ppu-strip vLaunchSELF.elf -o test_stripped.elf
ls -la test_stripped.elf

# Create first SELF using the same command as PKG
echo "2. Creating SELF with PKG command..."
/usr/local/ps3dev/bin/make_self_npdrm test_stripped.elf test_self_1 UP0001-HLFB00001_00-0000000000000000 2>/dev/null || /usr/local/ps3dev/bin/make_self test_stripped.elf test_self_1
ls -la test_self_1

# Create second SELF by copying the exact PKG process
echo "3. Copying from PKG EBOOT.BIN..."
cp build/pkg/USRDIR/EBOOT.BIN test_self_2
ls -la test_self_2

# Compare files
echo "4. Comparing files..."
echo "Size comparison:"
ls -la test_stripped.elf test_self_1 test_self_2

echo "Differences:"
diff test_self_1 test_self_2 && echo "Files are identical" || echo "Files are different"

echo "Hex dump comparison:"
echo "=== test_self_1 ==="
hexdump -C test_self_1 | head -3
echo "=== test_self_2 ==="
hexdump -C test_self_2 | head -3

# Clean up
rm -f test_self_1 test_self_2 test_stripped.elf