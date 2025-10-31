# Build & Install OBSBOT Control

Step-by-step guide for getting the GUI and CLI running on your Linux machine.

## TL;DR (safe defaults)
```bash
git clone https://github.com/bloopybae/obsbot-control-linux.git
cd obsbot-control-linux
./build.sh install --confirm
```

The installer checks dependencies, builds the project, installs binaries to `~/.local/bin`, drops a desktop launcher/icon, and keeps your camera available for other apps while you test.

## Requirements

### Toolchain
- CMake **3.16+**
- GCC **7+** or Clang **5+** (C++17)
- Qt 6: `qt6-base`, `qt6-multimedia` (development headers/libraries)
- `pkg-config` / `pkgconf`

### Recommended extras
- `lsof` (camera usage detection)
- `v4l2loopback` + `v4l-utils` if you plan to use the virtual camera feature

### Install commands
```bash
# Arch / Manjaro
sudo pacman -S base-devel cmake qt6-base qt6-multimedia pkgconf lsof
sudo pacman -S v4l2loopback-dkms v4l-utils    # optional virtual camera

# Debian / Ubuntu
sudo apt update
sudo apt install build-essential cmake qt6-base-dev qt6-multimedia-dev pkg-config lsof
sudo apt install v4l2loopback-dkms v4l2loopback-utils v4l-utils   # optional

# Fedora / RHEL
sudo dnf groupinstall "Development Tools"
sudo dnf install cmake qt6-qtbase-devel qt6-qtmultimedia-devel pkgconfig lsof
sudo dnf install v4l2loopback v4l-utils   # optional
```

## Using the build script
`build.sh` is the fastest and safest way to stay current.

```bash
./build.sh build --confirm        # compile only
./build.sh install --confirm      # build + install (default path: ~/.local/bin)
./build.sh clean --confirm        # remove build artifacts
./build.sh help                   # list every command/flag
```

Features:
- Detects missing dependencies and prints distro-friendly install commands.
- Keeps a dry-run mode (`./build.sh install`) so you see every action before confirming.
- Installs the GUI (`obsbot-gui`) and optional CLI (`obsbot-cli`).
- Adds `obsbot-control.desktop` and icon assets so the launcher shows up in your menu.
- Offers to extend your PATH if `~/.local/bin` is missing.

### Custom install location
Set XDG paths before running the installer:
```bash
export XDG_BIN_HOME="$HOME/bin"
export XDG_DATA_HOME="$HOME/.local/share"
./build.sh install --confirm
```

## Manual build (if you enjoy toggling switches yourself)
```bash
git clone https://github.com/bloopybae/obsbot-control-linux.git
cd obsbot-control-linux
mkdir build && cd build
cmake ..
make -j"$(nproc)"
```

Run straight from `build/`:
```bash
./obsbot-gui
./obsbot-cli --help
```

Manual install steps mirror what the script does:
```bash
install -Dm755 obsbot-gui ~/.local/bin/obsbot-gui
install -Dm755 obsbot-cli ~/.local/bin/obsbot-cli
install -Dm644 ../obsbot-control.desktop ~/.local/share/applications/obsbot-control.desktop
install -Dm644 ../resources/icons/camera.svg \
  ~/.local/share/icons/hicolor/scalable/apps/obsbot-control.svg
update-desktop-database ~/.local/share/applications
```

## Virtual camera setup
The repo ships a systemd unit (`resources/systemd/obsbot-virtual-camera.service`) and modprobe config to keep the virtual camera consistent.

```bash
sudo systemctl enable --now obsbot-virtual-camera.service
# or load it manually when you need it
sudo modprobe v4l2loopback video_nr=42 card_label="OBSBOT Virtual Camera" exclusive_caps=1
```

Adjust `video_nr`/`card_label` to match your environment and the device path configured in the app.

## Uninstall
```bash
./uninstall.sh --confirm
```
Removes binaries, desktop launcher, icon, and configuration (unless the config directory contains untracked files). PATH edits you made by hand stay untouched.

Manual cleanup:
```bash
rm -f ~/.local/bin/obsbot-{gui,cli}
rm -f ~/.local/share/applications/obsbot-control.desktop
rm -f ~/.local/share/icons/hicolor/scalable/apps/obsbot-control.svg
rm -rf ~/.config/obsbot-control
update-desktop-database ~/.local/share/applications
```

## Troubleshooting

### “Could not find Qt6”
Make sure the development headers are installed (`qt6-base-dev` / `qt6-qtbase-devel`). If you installed Qt6 from a custom location, export `Qt6_DIR` before running CMake:
```bash
cmake -DQt6_DIR=/opt/Qt/6.6.0/gcc_64/lib/cmake/Qt6 ..
```

### Linker can’t find `-ldev`
The proprietary OBSBOT SDK ships inside `sdk/lib/`. Ensure those files exist after cloning. If you used shallow clones or a mirror, pull again without filtering.

### Binary won’t start
```bash
ldd ~/.local/bin/obsbot-gui          # check for missing Qt libraries
~/.local/bin/obsbot-gui 2>&1 | tee gui.log
```
Look for Qt plugin errors or permission issues; ensure the binary is executable.

### Desktop launcher missing
Refresh your desktop cache:
```bash
update-desktop-database ~/.local/share/applications
gtk-update-icon-cache ~/.local/share/icons/hicolor
```
Some lightweight WMs ignore `.desktop` files; launch `obsbot-gui` manually in that case.

### Colors show escape codes in the script
You’re likely on an older checkout. Update the repo (`git pull`) and rerun the script.

### Need debug symbols
```bash
mkdir build-debug && cd build-debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . -j"$(nproc)"
```

## Getting help
- File issues: <https://github.com/bloopybae/obsbot-control-linux/issues>
- Architecture docs: `docs/adr/001-application-architecture.md`
- Usage overview: `../README.md`

If you’re stuck, include distro details (`/etc/os-release`), the build log, and `cmake --version` in your issue to speed up turnaround.
