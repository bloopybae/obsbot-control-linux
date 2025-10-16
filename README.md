# OBSBOT Meet 2 Control for Linux

A native Qt6 application for controlling OBSBOT Meet 2 cameras on Linux. Provides full camera control with an intuitive GUI while allowing simultaneous use with streaming/conferencing software.

## Features

### Camera Control
- **Pan/Tilt/Zoom (PTZ)** - Precise camera positioning with center reset
- **Auto-Framing** - AI-powered face tracking with single/upper body modes
- **Image Settings** - Brightness, contrast, saturation with auto/manual modes
- **Field of View** - Wide (86°), Medium (78°), Narrow (65°)
- **HDR** - High Dynamic Range for better exposure
- **Face AE/Focus** - Face-based auto exposure and auto focus
- **White Balance** - Multiple presets (auto, daylight, fluorescent, etc.)

### Live Preview
- **Camera Preview** - Real-time video preview with automatic aspect ratio detection
- **Usage Detection** - Warns when camera is in use by other applications (Chrome, OBS, Zoom)
- **Resource Management** - Automatically releases camera when not needed
- **Intelligent Layout** - Window expands only when preview successfully opens

### System Integration
- **System Tray** - Minimize to tray, click to restore
- **Start Minimized** - Optional startup directly to system tray
- **Auto-Reconnect** - Reconnects to camera when window is restored
- **State Preservation** - Remembers preview state across hide/show cycles

### Configuration
- **Persistent Settings** - All camera settings saved to XDG-compliant config file
- **Startup Restore** - Camera returns to saved state on connection
- **Manual Control** - Config file is human-readable and editable
- **Validation** - Helpful error messages for invalid configuration

## Why This Matters

The OBSBOT Meet 2 has excellent Linux UVC support, but lacks native control software. This application:

1. **Simultaneous Use** - Control camera settings while OBS/Chrome streams video
2. **Resource Friendly** - Releases camera when minimized/hidden
3. **Native Performance** - Qt6 application, not Electron
4. **Standard Compliance** - Uses XDG config directories, system tray

## Requirements

### System
- Linux (tested on Arch Linux with KDE Plasma)
- OBSBOT Meet 2 camera (USB connection)
- System tray support (KDE, GNOME, etc.)

### Build Dependencies
- Qt6 (Core, Widgets, Multimedia)
- CMake 3.16+
- C++17 compiler (GCC/Clang)
- OBSBOT SDK (included in `sdk/` directory)

### Runtime Dependencies
- Qt6 libraries
- V4L2 (Video4Linux2) support
- `lsof` for camera usage detection (optional but recommended)

## Building

```bash
# Clone the repository
git clone https://github.com/aaronsb/obsbot-controls-qt-linux.git
cd obsbot-controls-qt-linux

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build
make -j$(nproc)

# Run
./obsbot-meet2-gui
```

## Installation

```bash
# After building, install system-wide (optional)
sudo make install

# Or run directly from build directory
./obsbot-meet2-gui
```

## Usage

### First Run
1. Connect OBSBOT Meet 2 via USB
2. Launch application
3. Camera connects automatically
4. Adjust settings as desired
5. Settings auto-save to `~/.config/obsbot-meet2-control/settings.conf`

### Camera Preview
- Click **"Show Camera Preview"** to enable live preview
- If another app is using the camera, you'll see a warning with the process name
- Close the blocking application and try again
- Preview automatically disabled when window is hidden/minimized

### System Tray
- Click **X** button to minimize to tray (doesn't quit)
- Click tray icon to show/hide window
- Right-click tray icon for menu
- Enable **"Start minimized to tray"** checkbox for startup behavior

### Workflow Example
Perfect for streaming/conferencing:

1. Start app (optionally minimized to tray)
2. Open OBS and select OBSBOT Meet 2 as video source
3. Click tray icon to show control window
4. Adjust zoom, framing, brightness during stream
5. Hide window to tray when not needed
6. Camera remains available to OBS the entire time

## Configuration

Settings are stored in: `~/.config/obsbot-meet2-control/settings.conf`

### Configuration Format
```ini
# Camera Settings
face_tracking=enabled
hdr=disabled
fov=wide
face_ae=enabled
face_focus=enabled
zoom=1.0
pan=0.0
tilt=0.0

# Image Controls
brightness_auto=enabled
brightness=128
contrast_auto=enabled
contrast=128
saturation_auto=enabled
saturation=128
white_balance=auto

# Application Settings
start_minimized=disabled
```

### Manual Editing
- Boolean values: `enabled`/`disabled`, `true`/`false`, `yes`/`no`, `1`/`0`
- FOV values: `wide`/`medium`/`narrow` or `0`/`1`/`2`
- Zoom: `1.0` to `2.0`
- Pan/Tilt: `-1.0` to `1.0` (0 is center)
- Brightness/Contrast/Saturation: `0` to `255`

## Technical Details

### Architecture
- **Control Interface (SDK)** - USB control endpoint for camera settings
  - Can be used simultaneously by multiple applications
  - Always available when camera is connected

- **Video Stream (V4L2)** - `/dev/video*` device for preview
  - Exclusive access (one app at a time)
  - Preview optional - controls work without it

### Camera Detection
- Automatically finds OBSBOT camera in video device list
- Uses `lsof` to detect which process has video device open
- Filters out own process (control and preview can coexist)

### Resource Management
When window is hidden/minimized:
1. Preview state is saved
2. Preview is disabled if enabled
3. Camera SDK handle is released
4. Other applications can now access camera

When window is shown/restored:
1. Reconnects to camera control interface
2. Waits 1 second for stability
3. Attempts to restore preview if it was enabled
4. Shows warning if preview fails (camera in use)

## Troubleshooting

### Camera not detected
- Check USB connection: `lsof /dev/video0` (adjust device number)
- Verify udev permissions: User should be in `video` group
- Check kernel messages: `dmesg | grep -i video`

### Preview shows wrong camera
- Application auto-detects OBSBOT by name
- If multiple cameras, OBSBOT is selected by description match
- Fallback uses Qt6 Multimedia default camera

### Can't enable preview
- Orange warning shows which process is using camera
- Close blocking application (Chrome, OBS, Zoom, etc.)
- Click "Show Camera Preview" again
- Note: Controls work without preview!

### Settings not saving
- Check config directory exists: `~/.config/obsbot-meet2-control/`
- Verify write permissions
- Look for validation errors in config dialog

### System tray icon not showing
- Verify system tray support (KDE, GNOME extension, etc.)
- Check Qt6 platform plugin: `export QT_QPA_PLATFORMTHEME=qt6ct`
- Some minimal window managers lack system tray

## Command Line Interface

A CLI tool is also included for automation/scripting:

```bash
./obsbot-meet2-cli
```

See CLI help for available commands.

## Project Structure

```
obsbot-controls-qt-linux/
├── src/
│   ├── gui/           # Qt6 GUI application
│   ├── cli/           # Command-line interface
│   └── common/        # Shared configuration code
├── sdk/               # OBSBOT SDK (proprietary)
├── resources/         # Icons and resources
└── CMakeLists.txt     # Build configuration
```

## Contributing

This is a personal project but contributions are welcome:
- Bug reports and feature requests: Open an issue
- Pull requests: Please discuss major changes first
- Code style: Follow existing Qt/C++ conventions

## Credits

- OBSBOT for the Meet 2 camera and SDK
- Qt Project for the excellent framework
- Linux community for V4L2 support

## License

See LICENSE file for details.

## Disclaimer

This is an unofficial third-party application. Not affiliated with or endorsed by OBSBOT.
The OBSBOT SDK is proprietary and subject to OBSBOT's license terms.

---

**Made with ❤️ for the Linux community**

*Because Linux users deserve native camera control too.*
