# ADR-001: Application Architecture and Design Patterns

**Status:** Accepted
**Date:** 2025-10-15
**Decision Makers:** Development Team

## Context

We are building control applications (GUI and CLI) for the OBSBOT Meet 2 camera using the official OBSBOT SDK. The camera requires specific command sequences, has transitional states, and supports numerous control parameters. We need an architecture that:

1. Provides responsive UI despite camera command latency
2. Maintains consistent state between applications
3. Allows easy addition of new camera controls
4. Follows Linux/Unix conventions for configuration
5. Separates concerns cleanly between UI and business logic

## Decision

### 1. Modular Architecture with Separation of Concerns

**Structure:**
```
src/
├── common/          # Shared code between CLI and GUI
│   └── Config.*     # Configuration management
├── gui/             # Qt6 GUI application
│   ├── MainWindow.*         # Main orchestrator
│   ├── CameraController.*   # Camera SDK abstraction
│   ├── TrackingControlWidget.*
│   ├── PTZControlWidget.*
│   └── CameraSettingsWidget.*
└── cli/             # CLI application
    └── meet2_test.cpp
```

**Responsibilities:**
- **CameraController**: All SDK communication, state management, command execution
- **Widgets**: UI only, no business logic, call controller methods
- **MainWindow**: Orchestrates widgets and modes
- **Config**: XDG-compliant persistent storage

### 2. Optimistic UI with Debounce Protection

**Problem:** Camera commands have latency (50-500ms), causing UI lag and state conflicts.

**Solution:**
- **Optimistic UI Updates**: Checkboxes/controls update immediately when clicked
- **Per-Widget Debounce Timers**: 1-second protection after user action blocks status timer updates
- **Global Settling Period**: 2-second freeze on state updates after connection/config application
- **Cached State**: Controller caches intended state during settling, returns cache instead of camera state

**Implementation:**
```cpp
// Widget debounce
m_userInitiated = true;
m_controller->enableAutoFraming(checked);
m_commandTimer->start(1000);  // Block updates for 1 second

// Controller settling
void beginSettling(int durationMs = 2000);
bool isSettling() const;
CameraState m_cachedState;  // Returned during settling
```

### 3. XDG-Compliant Configuration System

**Location:** `$XDG_CONFIG_HOME/obsbot-meet2-control/settings.conf`
**Format:** Simple `key=value` with `#` comments

**Features:**
- Validation with detailed error reporting
- Three recovery options: Ignore (don't save), Reset to Defaults, Try Again
- Shared between CLI and GUI
- Auto-save on application exit (GUI)
- Apply on startup (both)

**Config Properties:**
- Camera: face_tracking, hdr, fov, face_ae, face_focus
- Position: zoom, pan, tilt
- Image: brightness, contrast, saturation, white_balance

### 4. Three-Mode Progressive UI Complexity

**Modes:**
1. **Simple**: Face tracking on/off only
2. **Advanced**: + PTZ controls (pan/tilt/zoom)
3. **Expert**: + All camera settings (HDR, FOV, image controls)

**Rationale:** Reduces cognitive load for basic users while providing full control for advanced users.

### 5. CLI Design: Non-Interactive by Default

**Default behavior:** Load config → Apply to camera → Exit
**Interactive mode:** `-i` flag enables menu-driven control

**Rationale:**
- Quick configuration without keeping app running
- Camera persists settings, so one-shot config is sufficient
- Enables scripting/automation

### 6. State Management Pattern

**Camera State Flow:**
```
User Action → Optimistic UI Update → Command to Camera → Debounce Timer
                                                                ↓
Status Timer (2s) ← State Update ← Camera Response ← [Wait for debounce]
```

**State Structure:**
```cpp
struct CameraState {
    // Tracking
    bool autoFramingEnabled;
    int aiMode, aiSubMode;

    // PTZ
    double pan, tilt, zoom;

    // Image
    bool hdrEnabled;
    int fovMode;
    bool faceAEEnabled, faceFocusEnabled;
    int brightness, contrast, saturation;
    int whiteBalance;

    // Status
    int zoomRatio, devStatus;
};
```

## Consequences

### Positive

✅ **Responsive UI**: Optimistic updates provide instant feedback
✅ **No State Conflicts**: Debounce + settling prevents race conditions
✅ **Consistent Behavior**: Same config/state model across CLI and GUI
✅ **Easy to Extend**: Adding new controls follows established patterns
✅ **Theme-Friendly**: No hardcoded colors, respects system theme
✅ **Standard-Compliant**: XDG config paths, follows Unix conventions

### Negative

❌ **Complexity**: Debounce/settling logic adds cognitive overhead
❌ **Delayed Feedback**: User must wait 1-2 seconds for camera confirmation
❌ **State Drift**: If commands fail silently, UI may not match camera

### Risks & Mitigations

**Risk:** Failed commands not visible to user
**Mitigation:** `commandFailed` signal shows error dialogs

**Risk:** Config file corruption
**Mitigation:** Validation with clear error messages and recovery options

**Risk:** SDK version changes break compatibility
**Mitigation:** SDK version logged on connection, wrapped in controller abstraction

## Implementation Notes

### Adding New Camera Controls

1. Add property to `Config::CameraSettings`
2. Update `Config::load()` parsing
3. Update `Config::save()` output
4. Add to `CameraController::CameraState`
5. Create setter in `CameraController`
6. Add UI control to appropriate widget
7. Add debounce timer handling in widget
8. Add to `applyConfigToCamera()` and `applyCurrentStateToCamera()`

### Face Tracking Quirk

Meet 2 requires two-step enable:
```cpp
cameraSetMediaModeU(MediaModeAutoFrame);
QTimer::singleShot(500ms, []{
    cameraSetAutoFramingModeU(AutoFrmSingle, AutoFrmUpperBody);
});
```

Disable is single-step:
```cpp
cameraSetMediaModeU(MediaModeNormal);
```

## References

- OBSBOT SDK Documentation (libdev_v2.1.0_7)
- XDG Base Directory Specification
- Qt6 Signal/Slot Documentation

## Status History

- 2025-10-15: Accepted
