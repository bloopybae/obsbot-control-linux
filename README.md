# OBSBOT Control for Linux

Modern Qt6 tools for OBSBOT cameras on Linux. Control every setting, keep the camera available to your streaming apps, and ship a virtual camera feed when you need it.

> Maintained by @bloopybae. Originally created by [Aaron Bockelie](https://github.com/aaronsb/obsbot-camera-control) — huge thanks to Aaron for pioneering the Linux support that made this fork possible.

## Highlights
- **Full PTZ + imaging control**: Auto-framing, HDR, FOV presets, face-aware exposure/focus, and manual sliders for brightness/contrast/saturation.
- **Preview when you want it**: Toggle the video preview, auto-detect conflicts, and release the device whenever the app hides or the preview stops.
- **Virtual camera pipeline**: Optional v4l2loopback integration with a built-in setup wizard for enabling/disabling the service without touching the terminal.
- **GPU-powered filters**: GLSL color grading that applies to both preview and the virtual camera output.
- **Fast configure**: Build/install script checks dependencies and keeps the GUI updated without extra packaging steps.
- **XDG-native config**: Resilient config storage with validation, manual editing, and automatic state restoration on connect.

## Why This Fork
Aaron’s original project proved the concept. This fork pushes further:
- Broader OBSBOT SDK coverage, including Tiny 2 family quirks and Meet 2 extras.
- Packaging/scripts tuned for day-to-day updates and local developer installs without external packaging systems.
- Consistent architecture shared between the GUI and internal tooling, plus modernized docs and better defaults for GPU filters and virtual camera workflows.

## Supported Cameras
- **OBSBOT Tiny 2 Family**: Fully verified.
- **OBSBOT Meet 2**: Fully verified.
- **OBSBOT Tiny 4K**: PTZ, HDR, manual image controls, white balance confirmed. Auto-framing and face AE/focus still firmware-limited.
- **Other models** (Tail Air, Me, etc.): SDK support exists but needs testing. [Open an issue](https://github.com/bloopybae/obsbot-control-linux/issues/new) with your findings.

## Installation
Make sure you have Qt 6 (Core/Widgets/Multimedia), CMake ≥ 3.16, a C++17 compiler, and `lsof` if you want camera-usage detection.

### Quick install (review first!)
```bash
git clone https://github.com/bloopybae/obsbot-control-linux.git
cd obsbot-control-linux
./build.sh install --confirm
```

What the script handles:
- Checks dependencies and prints distro-specific install commands.
- Builds the GUI binary (developer CLI optional).
- Installs to `~/.local/bin`, adds desktop launcher/icon, and offers PATH updates.

Common commands:
```bash
./build.sh build --confirm     # build without installing
./build.sh install --confirm   # build + install
./build.sh help                # discover all options
./uninstall.sh --confirm       # remove binaries and desktop assets
```

Prefer a manual build? Follow the steps in `docs/BUILD.md`.

## Using the App
- **First launch**: Plug in your OBSBOT, start the app, tweak settings, and they persist to `~/.config/obsbot-control/settings.conf`.
- **Preview mode**: Click “Show camera preview.” If another process owns the device, the app lists it so you can free the camera. Preview auto-disables when the window hides.
- **Filters**: Apply and tune GLSL color presets (Grayscale, Sepia, Invert, Warm, Cool) that immediately affect both preview and virtual camera output.
- **Virtual camera**: Optional systemd unit and modprobe config ship with the repo. Enable the service or run `sudo modprobe v4l2loopback video_nr=42 card_label="OBSBOT Virtual Camera" exclusive_caps=1`, then toggle the virtual camera inside the app.
- **Virtual camera**: Launch the “Set Up Virtual Camera” wizard for one-click install/enable/disable of the v4l2loopback service (uses PolicyKit). You can still copy the commands manually if you prefer.
- **Tray workflow**: Closing the window drops it to the tray. Reopen, tweak mid-stream, hide again without stealing camera access from OBS/Chrome/Meet.

## Documentation
- `docs/BUILD.md` — dependency breakdown, distro-specific instructions, manual build flow, troubleshooting.
- `docs/CONTRIBUTING.md` — patterns for adding new controls and working with the architecture.
- `docs/ROADMAP.md` — current status, planned enhancements, and technical debt tracking.
- `docs/adr/001-application-architecture.md` — architecture decisions for the GUI and internal tooling.

## Releases
- Versioned with SemVer. Tag `vMAJOR.MINOR.PATCH` on `main` when you are ready to ship.
- GitHub Actions (`.github/workflows/release.yml`) builds the project, packages an AppImage, generates checksums, and uploads artifacts to the tagged GitHub Release automatically.
- Release assets include the virtual camera service templates so users can enable the feature through the app’s “Set Up Virtual Camera” wizard—no CLI required unless they prefer it.

## Contributing & Support
- File bugs and feature ideas at `https://github.com/bloopybae/obsbot-control-linux/issues`.
- Pull requests welcome; please sync on larger architectural changes ahead of time.
- Follow the Qt/C++ conventions in `src/` and the debounced command patterns documented in `docs/CONTRIBUTING.md`.

## Credits
- Aaron Bockelie for the original Linux OBSBOT controller concept and early implementation.
- OBSBOT for making the SDK available.
- The Qt Project and the wider Linux community for the ecosystems this app relies on.

## License & SDK notice
- Code in `src/` and supporting scripts: MIT (see `LICENSE`).
- OBSBOT SDK under `sdk/`: proprietary binaries provided by OBSBOT; review their licensing before distributing.

## Disclaimer
Community-maintained project, not affiliated with or endorsed by OBSBOT. Use at your own risk, especially when redistributing the SDK.
