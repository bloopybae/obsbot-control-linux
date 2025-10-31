# Release Notes

## v1.0.7 — 2025-10-31
- Reworked `scripts/package-appimage.sh` to bootstrap AppDir installs, pin Qt6 discovery, and fail fast when JPEG XR runtime prerequisites are missing, ensuring reproducible AppImage builds.
- Taught the GitHub release workflow to install Qt SVG and JPEG XR packages so CI packaging succeeds without manual intervention.
- Documented the maintainer release checklist and linked it from the README to make future releases more repeatable.

## Earlier Releases
- v1.0.6 and prior: see GitHub release history.
