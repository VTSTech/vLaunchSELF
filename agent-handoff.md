# vLaunchSELF Agent Handoff Document

## Project Overview
vLaunchSELF is a PS3 file browser application with ELF/SELF launching capabilities, built using the PSL1GHT toolchain for PlayStation 3 homebrew development.

## Current Status (v1.0.0)

### ✅ **Completed Features**
- **Device Selection Menu**: User can select which PS3 device to browse from available devices
- **Multi-device Support**: Detects and displays all available PS3 devices
- **Interactive Device Selection**: Navigate device list with UP/DOWN buttons, select with CROSS
- **File Navigation**: Browse directories with full path display
- **ELF/SELF Launching**: Launch executables directly from file browser
- **Responsive UI**: Resolution-independent scaling for different display modes
- **Debounced Controls**: Prevents input lag and accidental button presses
- **Build System**: Complete PKG creation for PS3 installation

### 🔧 **Technical Implementation**
- **Language**: C with PSL1GHT SDK
- **Graphics**: tiny3d for rendering, libfont for text
- **File System**: PS3 sysfs API with proper error handling
- **Device Detection**: Scans /dev_bdvd, /dev_hdd0, /dev_hdd1, /dev_flash, /dev_rewrite
- **Build Tools**: make, make_self_npdrm, pkg utilities
- **Target**: PS3 (both real hardware and RPCS3 emulator)

### 🎮 **User Interface**
- **Title**: "vLaunchSELF v1" with Material Design color scheme
- **Device Selection**: Cyan highlighting for selected device
- **File Browser**: Clean directory listing with folder indicators
- **Controls**: Intuitive button mapping with on-screen help

### 📦 **Build System**
- **Targets**: `make`, `make elf`, `make stripped`, `make self`, `make pkg`
- **Output**: PKG file for installation, SELF file for direct boot
- **Signing**: NPDRM signing for real PS3 compatibility
- **Structure**: Organized build directory with proper artifact management

## 🚀 **Future Improvements (Agent Development Tasks)**

### Device Selection Enhancements
- [ ] **Device Type Descriptions**: Show device type and purpose during selection (e.g., "Main Hard Drive", "System Flash", "Optical Drive")
- [ ] **Device Information Display**: Show free space, total space, and filesystem type for each device
- [ ] **Visual Indicators**: Add icons or color coding for mounted/unmounted devices
- [ ] **Quick Device Switching**: Allow switching devices from within file browser without restarting
- [ ] **Network Device Support**: Add support for PS3 network storage devices if available
- [ ] **Device Favorites**: Allow users to bookmark frequently used devices

### File Browser Enhancements
- [ ] **File Size Display**: Show file sizes in human-readable format (KB, MB, GB)
- [ ] **File Modification Date**: Display last modified timestamp for files
- [ ] **File Type Icons**: Add visual indicators for different file types (ELF, SELF, folders, etc.)
- [ ] **Search Functionality**: Add filename search/filter capability
- [ ] **Sort Options**: Allow sorting by name, date, size, type
- [ ] **File Preview**: Basic preview capability for common file types

### User Experience Improvements
- [ ] **Progress Indicators**: Show loading progress for large directories
- [ ] **Error Handling**: Improve error messages and recovery options
- [ ] **Keyboard Input Support**: Add PS3 keyboard support for text input
- [ ] **Favorites/Bookmarks**: Allow saving favorite directories for quick access
- [ ] **Context Menu**: Right-click style menu for file operations (copy, delete, rename)
- [ ] **File Operations**: Add copy, move, delete functionality for files

### Advanced Features
- [ ] **File Hashing**: Display SHA1/MD5 hashes for file verification
- ] **Batch Operations**: Select multiple files for operations
- [ ] **File Compression**: Built-in zip/unzip functionality
- [ ] **Network Integration**: Browse network shares and transfer files
- [ ] **Theme Support**: Multiple UI color schemes and themes
- [ ] **Configuration Files**: Save user preferences and settings

### Technical Improvements
- [ ] **Memory Optimization**: Reduce memory usage for large directories
- [ ] **Performance Monitoring**: Add performance metrics and optimization
- [ ] **Logging System**: Comprehensive logging for debugging
- [ ] **Plugin System**: Allow third-party extensions and themes
- [ ] **Multi-language Support**: Internationalization for different languages
- [ ] **Accessibility**: Screen reader support and accessibility features

## 🔍 **Known Issues & Limitations**

### Current Limitations
- **Device Selection UI**: Not optimized for many devices (max 5 supported)
- **No Device Descriptions**: Device names only, no type information shown
- **File Count Limit**: Maximum 256 files per directory (MAX_FILES)
- **Path Length Limit**: Fixed 1024 character path limit (MAX_PATH)
- **No Network Support**: Currently only supports local PS3 devices

### Build Considerations
- **SELF File Issues**: Standalone SELF files may have compatibility issues (PKG works reliably)
- **Cross-Platform**: Build system assumes Linux/PS3DEV environment
- **Dependency Management**: Requires specific versions of PSL1GHT and related tools

## 🛠️ **Development Guidelines**

### Code Style
- Follow existing code style and naming conventions
- Use proper error handling for all sysfs operations
- Implement defensive programming for edge cases
- Add comprehensive comments for complex operations

### Testing Requirements
- Test on both real PS3 hardware and RPCS3 emulator
- Verify device detection with various PS3 configurations
- Test file launching with different ELF/SELF types
- Validate UI scaling across different display resolutions

### Build Process
- Always test build process after code changes
- Ensure both SELF and PKG targets work correctly
- Verify proper signing and compatibility
- Update version numbers and changelog for releases

## 📞 **Contact Information**
- **Developer**: VTSTech
- **GitHub**: github.com/VTSTech
- **Website**: www.VTS-Tech.org
- **Project**: PS3 Homebrew Development

## 🎯 **Next Steps for Agent**
1. **Prioritize**: Start with device selection enhancements (most user-visible improvements)
2. **Implement**: File size display and modification dates (high-utility features)
3. **Extend**: Add search functionality and favorites (power-user features)
4. **Optimize**: Performance improvements and memory optimization
5. **Expand**: Network support and advanced file operations

---

*This document serves as a comprehensive handoff guide for continued development of vLaunchSELF. Focus on user experience improvements while maintaining compatibility with existing PS3 hardware and software ecosystems.*