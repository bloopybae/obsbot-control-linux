# OBSBOT Control - Roadmap

## Current Status

### ✅ Completed Features

- [x] CLI tool with interactive and non-interactive modes
- [x] Qt6 GUI application with three UI complexity modes
- [x] Face tracking control (two-step enable/disable)
- [x] PTZ controls (pan/tilt/zoom with absolute positioning)
- [x] Advanced camera settings (HDR, FOV, Face AE, Face Focus)
- [x] XDG-compliant configuration system
- [x] Config validation with error recovery
- [x] Optimistic UI with debounce protection
- [x] Settling period for initialization
- [x] Theme-friendly UI (works with light/dark themes)
- [x] Separation of concerns architecture
- [x] Documentation (ADR-001)

### 🚧 In Progress

- [ ] Image controls (brightness, contrast, saturation, white balance)
  - [x] Config layer (parsing, validation, saving)
  - [ ] CameraState struct updates
  - [ ] CameraController methods
  - [ ] UI widgets (sliders + dropdown)
  - [ ] Integration with apply/save logic

## Planned Features

### High Priority

1. **Image Controls Completion**
   - Add brightness/contrast/saturation sliders (0-255 range)
   - Add white balance dropdown (Auto, Daylight, Fluorescent, etc.)
   - Wire up to camera SDK methods
   - Test with debounce/settling behavior

2. **Error Handling Improvements**
   - Show command failure details in UI
   - Retry mechanism for failed commands
   - Connection status indicator

3. **State Verification**
   - Verify camera state matches UI after settling period
   - Show warning if state drift detected

### Medium Priority

4. **Additional Camera Controls**
   - Sharpness (if available in SDK)
   - Exposure mode selection
   - Manual exposure/ISO controls
   - Backlight compensation

5. **Presets System**
   - Save/load custom presets
   - Quick-switch between presets
   - Per-application presets (e.g., "Video Call", "Recording", "Presentation")

6. **UI Enhancements**
   - Keyboard shortcuts
   - Reset to defaults button
   - Live preview of settings (if possible via SDK)

### Low Priority

7. **Advanced Features**
   - Gesture control (if SDK supports)
   - Auto-framing sensitivity adjustment
   - Multi-camera support
   - Systemd service for auto-apply on boot

8. **Developer Experience**
   - Unit tests for Config class
   - Integration tests for CameraController
   - Continuous Integration setup
   - Build instructions for different distros

## Known Issues

### Bugs

- None currently reported

### Limitations

- Camera state updates every 2 seconds (SDK limitation)
- No real-time preview in GUI
- White balance "Fine" mode might not be available on all firmware versions

## Technical Debt

- Consider caching SDK ranges (brightness/contrast min/max) instead of hardcoding 0-255
- Refactor widget creation in CameraSettingsWidget (getting long)
- Add const correctness to Config methods
- Consider moving debounce logic to a reusable base class

## Future Considerations

- Support for other OBSBOT cameras (Tiny 2, Tail Air)
- D-Bus interface for desktop integration
- KDE/GNOME control center integration
- Web interface for remote control
- Scriptable API (JSON-RPC or similar)

---

Last Updated: 2025-10-15
