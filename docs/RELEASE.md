# Release Checklist

Steps for cutting and publishing a new version of OBSBOT Control for Linux.

## 1. Prep your environment

Ensure you have the runtime dependencies that the AppImage bundler needs:

```bash
# Arch / Manjaro
sudo pacman -S jxrlib

# Debian / Ubuntu
sudo apt-get install libjxr-dev libqt6svg6-dev
```

You also need a Release build in `build/`:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j"$(nproc)"
```

## 2. Generate the release artifacts

Run the AppImage packaging script. It installs into an AppDir, bundles Qt
dependencies, and emits artifacts plus SHA256 sums.

```bash
scripts/package-appimage.sh "vX.Y.Z"
```

If the script complains about `libjxrglue.so`, install the JPEG XR runtime as
described above and rerun.

Verify the outputs in `build/artifacts/`:

```
obsbot-control-linux-vX.Y.Z-x86_64.AppImage
obsbot-control-linux-vX.Y.Z-x86_64.AppImage.sha256
```

## 3. Smoke test the AppImage

Execute the freshly built AppImage to validate the runtime:

```bash
./build/artifacts/obsbot-control-linux-vX.Y.Z-x86_64.AppImage --appimage-extract-and-run
```

Confirm the GUI launches, connects to a camera, and that optional features
(preview, filters, virtual camera wizard) behave as expected.

## 4. Tag and trigger the release workflow

Update `docs/RELEASE_NOTES.md` with the headline changes for the release, polish any other release notes, then tag from `main` using SemVer:

```bash
git tag vX.Y.Z
git push origin vX.Y.Z
```

Pushing the tag fires `.github/workflows/release.yml`, which rebuilds on Ubuntu,
packages with the same script, signs the artifacts, and publishes everything to
the GitHub Release page automatically.

Monitor the workflow run to ensure it completes successfully. The published
release will include:

- `obsbot-control-linux-vX.Y.Z-x86_64.AppImage`
- `obsbot-control-linux-vX.Y.Z-x86_64.AppImage.sha256`
- systemd/modprobe assets required by the virtual camera wizard

## 5. Post-release

1. Announce the release (community channels, issue tracker, etc.).
2. Bump development version references if needed.
3. Capture follow-up tasks in `docs/ROADMAP.md` or GitHub issues.
