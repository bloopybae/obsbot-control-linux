# Contributing to OBSBOT Control

Thanks for helping push Linux support for OBSBOT hardware forward! This guide explains how the project is structured and how to add new features safely.

## Before you start
- Read the architecture notes in `docs/adr/001-application-architecture.md`.
- Build the project locally with `./build.sh install --confirm` and verify the GUI runs. Enable the developer CLI with `./build.sh build --with-cli --confirm` if you need it for testing.
- Format code to match the surrounding C++ style (Qt signal/slot patterns, brace placement, etc.).
- Every new behavior should have manual test notes or automated coverage where practical.

## Project layout (quick refresher)
```
src/
  common/        // shared config/state types
  gui/           // Qt widgets, controllers, main window
  cli/           // one-shot + interactive command line tool
sdk/             // bundled OBSBOT SDK binaries/headers
resources/       // icons, desktop files, systemd units
```

Key classes:
- `Config` (`src/common/Config.*`): owns persisted settings and validation.
- `CameraController` (`src/gui/CameraController.*`): wraps SDK commands with caching, debounce and error handling.
- `CameraSettingsWidget`, `PTZControlWidget`, `TrackingControlWidget`: UI surfaces for specific control groups.
- CLI entry point (`src/cli/meet2_test.cpp`): developer-only tool that loads config, applies settings, or runs an interactive menu.

## Adding a new camera control (example workflow)

1. **Config layer**
   - Append the new field to `Config::CameraSettings` in `src/common/Config.h`.
   - Initialize defaults in `Config::setDefaults()` (`src/common/Config.cpp`).
   - Extend the parser in `Config::load()` to read and validate the key.
   - Include it in `Config::save()` output with comments to aid manual editing.

2. **Camera controller**
   - Add the field to `CameraController::CameraState` in `src/gui/CameraController.h`.
   - Implement a setter that wraps the SDK call in `CameraController.cpp`, using `executeCommand()` for consistent logging and retry behavior.
   - Update `updateState()`, `applyConfigToCamera()`, and `applyCurrentStateToCamera()` so cached state stays in sync.

3. **GUI widgets**
   - Choose the appropriate widget (`CameraSettingsWidget`, `PTZControlWidget`, etc.).
   - Create the control, connect signals, and respect the debounce timer pattern (`m_commandTimer` + `m_userInitiated` flags).
   - Populate initial values in `loadConfiguration()` (MainWindow) and push user changes into `CameraController`.

4. **CLI support (optional for developer tooling)**
   - Expose the setting inside `applyConfigToCamera()` in `src/cli/meet2_test.cpp`.
   - Update any interactive prompts if the feature should be togglable via CLI.

5. **Documentation**
   - Update `README.md` and relevant docs to mention the new capability.
   - Add troubleshooting notes if the control behaves differently across camera models.

6. **Testing checklist**
   - Build the project (`cmake --build build -j"$(nproc)"`).
   - Exercise the new widget in the GUI; confirm state persists across reconnects.
   - Validate config save/load with both default and custom values.
   - Run the CLI in both one-shot and `--interactive` modes if you modified developer tooling.

## Submitting changes
- Keep commits focused; split refactors and feature work if possible.
- Describe manual testing in your PR body (`Test Plan:` section).
- Link to related issues or roadmap items.
- Expect questions about camera coverage—if you only tested on a single model, say so.

## Issue/PR labels
- `bug`, `enhancement`, `camera-support`, `docs`, `build`, `cli`, `gui` help triage incoming work.
- If you’re unsure which label fits, leave it blank and a maintainer will adjust.

## Code of conduct
Be respectful. We all want better Linux camera tooling. Discrimination, harassment, or dismissive behavior isn’t tolerated.
