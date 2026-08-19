# vLaunchSELF Changelog

## [Unreleased]

## [1.0.0] - 2024-12-10

### Added
- **Device Selection Menu**: User can now select which device to browse from available devices
- **Multi-device Support**: Detects and displays all available PS3 devices
- **Interactive Device Selection**: Navigate device list with UP/DOWN buttons, select with CROSS
- **Enhanced Device Detection**: Scans all 5 PS3 device types in priority order
- **User Choice**: No longer auto-selects "best" device - user decides which to explore
- **Visual Feedback**: Selected device highlighted in cyan during selection
- **Device Status Display**: Shows which devices are available/not available during scan
- **File Size Display**: Shows file sizes in human-readable format (B, KB,MB, GB)
- **File Timestamps**: Displays last modified date and time in MM-DD-YYYY HH:MM format
- **Device Re-selection**: Press Triangle at device root to show device selection menu again
- **Enhanced File List**: Improved file listing with size and time information

### Changed
- **Version Bump**: Updated from v0 to v1.0.0
- **Title Update**: Shows "vLaunchSELF v1" in application title
- **Device Selection Logic**: From auto-select to user-controlled selection
- **Initial Screen**: Now shows device selection when multiple devices available
- **Navigation**: Enhanced input handling for device selection mode

### Fixed
- **Device Detection**: Now properly scans all devices instead of stopping at first found
- **Build Issues**: Fixed compilation warnings and function declaration order
- **SELF File Creation**: Improved build process consistency

### Technical Details
- **Device Scan Order**: /dev_bdvd → /dev_hdd0 → /dev_hdd1 → /dev_flash → /dev_rewrite
- **Selection Controls**: UP/DOWN to navigate, CROSS to select, CIRCLE to exit
- **Device Display**: Shows available devices with indexed list [0], [1], [2], etc.
- **Fallback Behavior**: If only one device available, uses it directly (no selection needed)
- **Error Handling**: Graceful handling when no devices found
- **File Size Formatting**: Automatic conversion to B/KB/MB/GB units
- **Timestamp Display**: MM-DD-YYYY HH:MM format with time information
- **Smart Navigation**: Triangle button context-aware (up vs device selection)
- **TitleID**: VTSTECH02 (PS3 application identifier)

## [0.1.0] - 2024-12-10

### Added
- Initial release of vLaunchSELF - ELF/SELF launcher for PS3
- Text rendering using 8x8 bitmap font
- Directory navigation with full path display
- ELF/SELF auto-launching capability
- Debounced button controls
- Display scaling for proper resolution handling
- Based on PS3-Moonlight graphics and text rendering system

### Features
- **Device Selection**: Choose from available PS3 devices to browse
- **File Navigation**: Navigate through directories using D-pad
- **File Launching**: Launch ELF and SELF files with X button
- **Directory Navigation**: Enter directories with X button, go up with Triangle
- **Exit**: Circle button to exit application
- **Multi-device Support**: Detects and displays all connected PS3 storage devices
- **Responsive UI**: Scales to different display resolutions

### Supported Devices
- `/dev_bdvd` - Blu-ray disc drive (if mounted)
- `/dev_hdd0` - First hard drive partition (main storage)
- `/dev_hdd1` - Second hard drive partition (if installed)
- `/dev_flash` - Flash memory (system files)
- `/dev_rewrite` - Rewrite area (if available)

### Controls

#### Device Selection Mode
- **X Button**: Select device
- **O Button**: Exit application
- **Up/Down D-pad**: Navigate between devices

#### File Browser Mode
- **X Button**: Enter directory / Launch ELF/SELF file
- **O Button**: Exit application
- **Triangle Button**: Go up one directory level OR show device selection at root
- **Up/Down D-pad**: Navigate through file list
- **Left/Right D-pad**: Navigate through file list (wrap around)

### Technical Details
- Built using PSL1GHT toolchain
- Cross-compiled with powerpc64-ps3-elf-gcc
- Uses tiny3d for graphics rendering
- Uses libfont for text rendering
- Implements proper PS3 sysfs file system access
- Includes debounced input handling
- Resolution-independent UI scaling

### Build System
- Makefile with targets: `make`, `make elf`, `make stripped`, `make self`, `make pkg`
- Automatic build directory creation
- PKG creation for PS3 installation
- SELF creation for direct booting

### Known Issues
- Device selection UI not optimized for many devices (max 5 supported)
- No device description/type shown during selection
- Limited to 256 files per directory (MAX_FILES)
- Fixed path length of 1024 characters (MAX_PATH)

### Dependencies
- PSL1GHT SDK at `/usr/local/ps3dev/`
- tiny3d graphics library
- libfont text rendering library
- Standard PS3 sys libraries (rsx, sysutil, etc.)



---

### Build Information
- **Version**: 1.0.0
- **Author**: VTSTech
- **License**: MIT
- **Target**: PS3 (RPCS3 and real hardware)
- **Build Tools**: PSL1GHT, make_self_npdrm, pkg utilities
- **APPID**: VTSTECH02
- **ContentID**: UP0001-VTSTECH02_00-0000000000000000

### Key Features in v1.0.0
- **User Device Selection**: Choose which PS3 device to explore
- **Multi-Device Support**: Detect and browse any available storage device
- **Interactive Selection Menu**: Navigate devices with controller
- **Enhanced Device Detection**: Comprehensive scanning of all PS3 device types
- **Improved User Experience**: Clear visual feedback and intuitive controls