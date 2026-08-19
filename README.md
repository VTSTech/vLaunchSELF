# vLaunchSELF - PS3 ELF/SELF Launcher

> A powerful file browser for PS3 with ELF/SELF launching capabilities!

<img width="1828" height="1079" alt="image" src="https://github.com/user-attachments/assets/28024660-020a-4896-8371-96c39a884bdf" />

## Features

- 📁 **File Browsing** - Navigate the PS3 file system with ease
- 🔧 **ELF/SELF Launching** - Execute homebrew executables directly
- 🎮 **Full Controller Support** - Intuitive navigation with button debouncing
- 📺 **Resolution Scaling** - Works on both 720p and 1080p displays
- 🧾 **Text Rendering** - Custom 8x8 bitmap font with smooth scaling

## Controls

| Button | Function |
|--------|----------|
| **X** | Enter directory / Launch ELF/SELF |
| **O** | Exit application |
| **Triangle** | Go to parent directory |
| **Up/Down** | Navigate file list |

## Building

### Prerequisites
- [PS3DEV](https://www.psdevwiki.com/ps3/Dev_Tools) environment
- PSL1GHT SDK installed
- `ppu-gcc` toolchain

### Build Commands

```bash
# Build PKG package
make pkg

# Build ELF executable
make

# Clean build artifacts
make clean
```

## Installation

1. Copy `build/vLaunchSELF.pkg` to your PS3
2. Install via XMB Package Manager
3. Launch from Games section
4. Navigate to any `.elf` or `.self` file to execute it

## Release History

| Version | Date | Description |
|---------|------|-------------|
| v0.1 | 2024 | Initial release |
| v0.2 | 2024 | Added scrolling, controls legend, footer credits |
| v0.3 | 2024 | NPDRM signing for real PS3 compatibility |

## Technical Details

- **Target ID**: HLFB00001
- **Title**: vLaunchSELF PS3
- **Format**: ELF/SELF launcher
- **Dependencies**: librsx, libtiny3d, libfont3d, libsysmodule, libsysfs

## Contributing

Contributions are welcome! Feel free to:
- Submit pull requests
- Report bugs
- Add new features
- Improve documentation

## License

This project is open source and free for homebrew development.

## Credits

**Author**: VTSTech  
**Website**: [www.VTS-Tech.org](https://www.vts-tech.org)  
**GitHub**: [github.com/VTSTech](https://github.com/VTSTech)  
**Documentation**: [PSL1GHT Documentation](https://github.com/VTSTech/PSL1GHT)

## Changelog

### v0.3
- ✅ NPDRM signing enabled for real PS3 compatibility
- ✅ Fixed PKG installation error 80029530
- ✅ Improved file browser navigation
- ✅ Added scrolling file list
- ✅ Button debouncing for smooth navigation

### v0.2
- ✅ ELF/SELF launching via sysProcessExitSpawn2
- ✅ Triangle button for parent directory
- ✅ Controls legend display
- ✅ Footer credits
- ✅ Scrolling file list support

### v0.1
- ✅ Initial release
- ✅ Basic file browsing
- ✅ Font rendering system
- ✅ Directory navigation

---

**Home PS3 Homebrew Development** | **PS3 File Browser** | **PSL1GHT Development**
