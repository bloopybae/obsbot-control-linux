#!/usr/bin/env bash
set -euo pipefail

PROJECT_ROOT=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
BUILD_DIR="${PROJECT_ROOT}/build"
APPDIR="${BUILD_DIR}/AppDir"
ARTIFACTS_DIR="${BUILD_DIR}/artifacts"
VERSION_TAG="${1:-dev}"

if [[ ! -d "${BUILD_DIR}" ]]; then
    echo "Build directory not found. Run cmake --build build first." >&2
    exit 1
fi

rm -rf "${APPDIR}" "${ARTIFACTS_DIR}"
mkdir -p "${APPDIR}" "${ARTIFACTS_DIR}"

echo "[packaging] Installing into AppDir"
cmake --install "${BUILD_DIR}" --prefix "${APPDIR}/usr"

DESKTOP_FILE="${APPDIR}/usr/share/applications/obsbot-control.desktop"
ICON_FILE="${APPDIR}/usr/share/icons/hicolor/scalable/apps/obsbot-control.svg"

if [[ ! -f "${DESKTOP_FILE}" ]]; then
    echo "Desktop file not found in AppDir" >&2
    exit 1
fi

if [[ ! -f "${ICON_FILE}" ]]; then
    cp "${PROJECT_ROOT}/resources/icons/camera.svg" "${ICON_FILE}"
fi

LINUXDEPLOY="${BUILD_DIR}/linuxdeploy-x86_64.AppImage"
LINUXDEPLOY_QT="${BUILD_DIR}/linuxdeploy-plugin-qt-x86_64.AppImage"

if [[ ! -f "${LINUXDEPLOY}" ]]; then
    curl -L "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage" \
        -o "${LINUXDEPLOY}"
    chmod +x "${LINUXDEPLOY}"
fi

if [[ ! -f "${LINUXDEPLOY_QT}" ]]; then
    curl -L "https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage" \
        -o "${LINUXDEPLOY_QT}"
    chmod +x "${LINUXDEPLOY_QT}"
fi

export APPIMAGE_EXTRACT_AND_RUN=1
QMAKE_BIN="$(command -v qmake6 || command -v qmake || true)"

if [[ -z "${QMAKE_BIN}" ]]; then
    echo "qmake/qmake6 not found. Install Qt development tools." >&2
    exit 1
fi

export PATH="$(dirname "${QMAKE_BIN}"):${PATH}"
export QMAKE="${QMAKE_BIN}"
export LD_LIBRARY_PATH="${PROJECT_ROOT}/sdk/lib:${APPDIR}/usr/lib:${LD_LIBRARY_PATH:-}"

if ! ldconfig -p 2>/dev/null | grep -q 'libjxrglue\.so' && [[ ! -e /usr/lib/libjxrglue.so* ]]; then
    cat >&2 <<'EOF'
Missing JPEG XR runtime (libjxrglue). Install the JPEG XR package for your distro (e.g.
  - Arch/Manjaro: sudo pacman -S jxrlib
  - Debian/Ubuntu: sudo apt-get install libjxr-dev
and rerun this script.
EOF
    exit 1
fi

QT_LIB_DIR="$("${QMAKE_BIN}" -query QT_INSTALL_LIBS || true)"
declare -a QT_EXTRA_MODULES=(
    "${QT_LIB_DIR}/libQt6Core.so.6"
    "${QT_LIB_DIR}/libQt6Gui.so.6"
    "${QT_LIB_DIR}/libQt6Widgets.so.6"
    "${QT_LIB_DIR}/libQt6Network.so.6"
    "${QT_LIB_DIR}/libQt6DBus.so.6"
    "${QT_LIB_DIR}/libQt6Multimedia.so.6"
    "${QT_LIB_DIR}/libQt6MultimediaWidgets.so.6"
    "${QT_LIB_DIR}/libQt6OpenGL.so.6"
    "${QT_LIB_DIR}/libQt6OpenGLWidgets.so.6"
    "${QT_LIB_DIR}/libQt6Svg.so.6"
)

QT_EXTRA_ARGS=()
for module in "${QT_EXTRA_MODULES[@]}"; do
    if [[ -f "${module}" ]]; then
        QT_EXTRA_ARGS+=("--extra-module=${module}")
    fi
done

pushd "${BUILD_DIR}" >/dev/null
rm -f obsbot-control-linux-*.AppImage

"${LINUXDEPLOY_QT}" --appdir "${APPDIR}" "${QT_EXTRA_ARGS[@]}"
"${LINUXDEPLOY}" --appdir "${APPDIR}" \
    --desktop-file "${DESKTOP_FILE}" \
    --icon-file "${ICON_FILE}" \
    --output appimage

APPIMAGE_SOURCE=$(ls -1t *.AppImage 2>/dev/null | head -n1)
if [[ -z "${APPIMAGE_SOURCE}" ]]; then
    echo "AppImage generation failed" >&2
    exit 1
fi

APPIMAGE_NAME="obsbot-control-linux-${VERSION_TAG}-x86_64.AppImage"
mv "${APPIMAGE_SOURCE}" "${ARTIFACTS_DIR}/${APPIMAGE_NAME}"
popd >/dev/null

(cd "${ARTIFACTS_DIR}" && sha256sum "${APPIMAGE_NAME}" > "${APPIMAGE_NAME}.sha256")

echo "[packaging] AppImage ready: ${ARTIFACTS_DIR}/${APPIMAGE_NAME}"
