# Contributing to OBSBOT Meet 2 Control

## Adding New Camera Controls

Follow these steps to add a new camera control parameter:

### 1. Update Config Layer

**File:** `src/common/Config.h`
```cpp
struct CameraSettings {
    // ... existing fields ...
    int yourNewControl;  // Add your field with comment
};
```

**File:** `src/common/Config.cpp`

a. Add to defaults:
```cpp
void Config::setDefaults() {
    // ... existing defaults ...
    m_settings.yourNewControl = defaultValue;
}
```

b. Add to required keys:
```cpp
std::vector<std::string> requiredKeys = {
    // ... existing keys ...
    "your_new_control"
};
```

c. Add parsing logic:
```cpp
} else if (key == "your_new_control") {
    // Parse and validate the value
    // Set m_settings.yourNewControl
}
```

d. Add to save():
```cpp
file << "# Your New Control Description\n";
file << "your_new_control=" << m_settings.yourNewControl << "\n\n";
```

### 2. Update CameraController

**File:** `src/gui/CameraController.h`

a. Add to CameraState:
```cpp
struct CameraState {
    // ... existing fields ...
    int yourNewControl;
};
```

b. Add setter method:
```cpp
bool setYourNewControl(int value);
```

**File:** `src/gui/CameraController.cpp`

a. Implement setter:
```cpp
bool CameraController::setYourNewControl(int value)
{
    if (!m_connected) return false;

    return executeCommand("Set Your Control", [this, value]() {
        return m_device->cameraSetYourControlR(value);
    });
}
```

b. Update `updateState()`:
```cpp
void CameraController::updateState() {
    // ... existing code ...
    m_currentState.yourNewControl = status.tiny.your_control;
}
```

c. Update `applyConfigToCamera()` and `applyCurrentStateToCamera()`:
```cpp
setYourNewControl(settings.yourNewControl);
```

d. Update `saveCurrentStateToConfig()`:
```cpp
settings.yourNewControl = m_currentState.yourNewControl;
```

### 3. Add UI Widget

**File:** `src/gui/CameraSettingsWidget.h`

a. Add widget member:
```cpp
private:
    QSlider *m_yourControlSlider;  // or QComboBox, QCheckBox, etc.
```

b. Add getter:
```cpp
public:
    int getYourControl() const { return m_yourControlSlider->value(); }
```

c. Add setter (for config initialization):
```cpp
void setYourControl(int value) {
    m_yourControlSlider->blockSignals(true);
    m_yourControlSlider->setValue(value);
    m_yourControlSlider->blockSignals(false);
}
```

**File:** `src/gui/CameraSettingsWidget.cpp`

a. Create widget in constructor:
```cpp
m_yourControlSlider = new QSlider(Qt::Horizontal, this);
m_yourControlSlider->setRange(minValue, maxValue);
connect(m_yourControlSlider, &QSlider::valueChanged,
        this, &CameraSettingsWidget::onYourControlChanged);
```

b. Add slot:
```cpp
private slots:
    void onYourControlChanged(int value);
```

c. Implement slot with debounce:
```cpp
void CameraSettingsWidget::onYourControlChanged(int value)
{
    m_userInitiated = true;
    m_controller->setYourNewControl(value);
    m_commandTimer->start(1000);
}
```

d. Update from state:
```cpp
void CameraSettingsWidget::updateFromState(const CameraController::CameraState &state)
{
    // ... existing code ...
    if (!m_userInitiated && !commandInFlight && !isSettling) {
        if (m_yourControlSlider->value() != state.yourNewControl) {
            m_yourControlSlider->blockSignals(true);
            m_yourControlSlider->setValue(state.yourNewControl);
            m_yourControlSlider->blockSignals(false);
        }
    }
}
```

### 4. Update MainWindow Config Init

**File:** `src/gui/MainWindow.cpp`

In `loadConfiguration()`:
```cpp
m_settingsWidget->setYourControl(settings.yourNewControl);
```

In `getUIState()`:
```cpp
state.yourNewControl = m_settingsWidget->getYourControl();
```

### 5. Update CLI (Optional)

**File:** `src/cli/meet2_test.cpp`

Add to `applyConfigToCamera()`:
```cpp
cout << "  Setting Your Control: " << settings.yourNewControl << endl;
ret = dev->cameraSetYourControlR(settings.yourNewControl);
```

### 6. Test

1. Build: `cmake --build build -j$(nproc)`
2. Run GUI: `./build/obsbot-meet2-gui`
3. Test control works
4. Test config save/load
5. Test with invalid config values
6. Test debounce behavior (rapid clicking)
7. Test settling period (click during connection)

## Architecture Guidelines

- **Widgets**: UI only, no business logic
- **Controller**: All SDK communication
- **Optimistic UI**: Update immediately, validate later
- **Debounce**: 1 second after user action
- **Settling**: 2 seconds after connection/config apply
- **Config**: All persistent state

## Code Style

- Use Qt naming conventions for Qt code
- Use camelCase for methods
- Use m_ prefix for member variables
- Add comments for non-obvious logic
- Keep methods small and focused

## Testing

- Test with both Breeze Light and Breeze Dark themes
- Test connection loss/reconnection
- Test rapid control changes
- Test config validation edge cases

## Documentation

- Update ADR if architectural decisions change
- Update ROADMAP.md when adding features
- Add inline comments for complex logic
