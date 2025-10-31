# OBSBOT Control Roadmap

Last updated: 2025-10-18

## Recently shipped
- ✅ GPU shader pipeline for preview + virtual camera.
- ✅ Image controls (brightness/contrast/saturation/white balance) with manual + auto modes.
- ✅ Virtual camera helper unit for v4l2loopback.
- ✅ Config validator with recovery prompts (ignore/reset/retry).
- ✅ CLI parity with GUI defaults, including interactive menu mode.

## Near-term focus
1. **Multi-camera awareness**
   - UI surfacing for multiple connected OBSBOT devices.
   - Remember preferred device by serial number.
2. **Preset management**
   - Save/load named profiles.
   - Optional auto-apply by workspace or app.
3. **Better failure visibility**
   - Toast/error panel for SDK command failures.
   - Retry queue with backoff for flaky USB connections.
4. **Tiny 2 feature parity**
   - LED brightness, microphone distance, and gesture toggles exposed in the GUI.

## Stretch goals
- Recording-friendly preview (no exclusive camera access) using gstreamer pipeline.
- D-Bus bridge for scripting and desktop integration.
- Web remote control with WebRTC preview.
- Continuous integration builds for main distros.

## Technical debt watchlist
- Consolidate debounce logic into a reusable helper shared by widgets.
- Extract SDK command wrappers to their own module for easier unit testing.
- Cache SDK capability ranges instead of using hard-coded slider spans.
- Improve CLI argument parsing (switch to `CLI11` or similar).

## How to help
- Try the latest changes and report camera compatibility in GitHub issues.
- Test virtual camera flows on your desktop environment and share findings.
- Grab any of the `help wanted` issues on GitHub—most map directly to items above.
