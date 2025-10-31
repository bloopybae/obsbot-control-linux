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
    wget -q "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage" -O "${LINUXDEPLOY}"
    chmod +x "${LINUXDEPLOY}"
fi

if [[ ! -f "${LINUXDEPLOY_QT}" ]]; then
    wget -q "https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage" -O "${LINUXDEPLOY_QT}"
    chmod +x "${LINUXDEPLOY_QT}"
fi

export APPIMAGE_EXTRACT_AND_RUN=1
export QMAKE="$(command -v qmake6 || command -v qmake || true)"

if [[ -z "${QMAKE}" ]]; then
    echo "qmake/qmake6 not found. Install Qt development tools." >&2
    exit 1
fi

pushd "${BUILD_DIR}" >/dev/null
rm -f ./*.AppImage
"${LINUXDEPLOY_QT}" --appdir "${APPDIR}" >/dev/null
"${LINUXDEPLOY}" --appdir "${APPDIR}" --desktop-file "${DESKTOP_FILE}" --icon-file "${ICON_FILE}" --output appimage >/dev/null

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
