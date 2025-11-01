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
export GST_PLUGIN_SYSTEM_PATH="${APPDIR}/usr/lib/gstreamer-1.0"
export GST_PLUGIN_SYSTEM_PATH_1_0="${APPDIR}/usr/lib/gstreamer-1.0"

declare -a LIB_SEARCH_PATHS=(
    "/usr/lib/x86_64-linux-gnu"
    "/usr/lib64"
    "/usr/lib"
    "/lib/x86_64-linux-gnu"
    "/lib64"
    "/lib"
)

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

copy_system_library() {
    local lib="$1"
    local dest_dir="$2"
    for base in "${LIB_SEARCH_PATHS[@]}"; do
        local candidate="${base}/${lib}"
        if [[ -f "${candidate}" ]]; then
            mkdir -p "${dest_dir}"
            cp -a "${candidate}" "${dest_dir}/"
            local resolved
            resolved="$(readlink -f "${candidate}" 2>/dev/null || true)"
            if [[ -n "${resolved}" ]] && [[ "${resolved}" != "${candidate}" ]]; then
                local resolved_name
                resolved_name="$(basename "${resolved}")"
                if [[ ! -f "${dest_dir}/${resolved_name}" ]]; then
                    cp -a "${resolved}" "${dest_dir}/"
                fi
            fi
            echo "${dest_dir}/$(basename "${lib}")"
            return 0
        fi
    done
    return 1
}

bundle_sdk_runtime() {
    local sdk_lib_src="${PROJECT_ROOT}/sdk/lib"
    local sdk_lib_dest="${APPDIR}/usr/lib"
    if [[ -d "${sdk_lib_src}" ]]; then
        echo "[packaging] Bundling OBSBOT SDK runtime libraries"
        mkdir -p "${sdk_lib_dest}"
        cp -a "${sdk_lib_src}/." "${sdk_lib_dest}/"
        while IFS= read -r lib; do
            [[ -f "${lib}" ]] || continue
            "${LINUXDEPLOY}" --appdir "${APPDIR}" --library "${lib}" >/dev/null
        done < <(find "${sdk_lib_dest}" -maxdepth 1 -type f -name 'lib*.so*')
    else
        echo "warning: OBSBOT SDK libraries not found at ${sdk_lib_src}" >&2
    fi

    if [[ -d "${PROJECT_ROOT}/sdk/include" ]]; then
        mkdir -p "${APPDIR}/usr/include"
        cp -a "${PROJECT_ROOT}/sdk/include/." "${APPDIR}/usr/include/"
    fi
}

bundle_gstreamer_assets() {
    local gst_plugin_dest="${APPDIR}/usr/lib/gstreamer-1.0"
    local gst_lib_dest="${APPDIR}/usr/lib"
    local qt_mediaservice_dest="${APPDIR}/usr/plugins/mediaservice"

    mkdir -p "${gst_plugin_dest}" "${gst_lib_dest}"

    local found_plugins=false
    for base in "${LIB_SEARCH_PATHS[@]}"; do
        if [[ -d "${base}/gstreamer-1.0" ]]; then
            echo "[packaging] Copying GStreamer plugins from ${base}/gstreamer-1.0"
            cp -a "${base}/gstreamer-1.0/." "${gst_plugin_dest}/"
            found_plugins=true
        fi
    done

    if [[ "${found_plugins}" == "false" ]]; then
        echo "error: No GStreamer plugin directories found on the system." >&2
        exit 1
    fi

    if [[ -d "/usr/share/gstreamer-1.0" ]]; then
        mkdir -p "${APPDIR}/usr/share/gstreamer-1.0"
        cp -a "/usr/share/gstreamer-1.0/." "${APPDIR}/usr/share/gstreamer-1.0/"
    fi

    local -a gst_core_libs=(
        "libgstreamer-1.0.so.0"
        "libgstbase-1.0.so.0"
        "libgstaudio-1.0.so.0"
        "libgstvideo-1.0.so.0"
        "libgstpbutils-1.0.so.0"
        "libgstapp-1.0.so.0"
        "libgstgl-1.0.so.0"
        "libgstplay-1.0.so.0"
        "libgstfft-1.0.so.0"
        "libgstwebrtc-1.0.so.0"
        "libgsttag-1.0.so.0"
        "libgstcodecparsers-1.0.so.0"
    )
    for lib in "${gst_core_libs[@]}"; do
        local copied_path=""
        if ! copied_path=$(copy_system_library "${lib}" "${gst_lib_dest}"); then
            echo "error: Required GStreamer library ${lib} not found on host." >&2
            exit 1
        else
            "${LINUXDEPLOY}" --appdir "${APPDIR}" --library "${copied_path}" >/dev/null
        fi
    done

    local -a helper_libs=(
        "liborc-0.4.so.0"
        "libjpeg.so.8"
        "libtheoradec.so.1"
        "libtheoraenc.so.1"
        "libogg.so.0"
        "libvorbis.so.0"
        "libvorbisenc.so.2"
        "libva.so.2"
        "libvpx.so.7"
        "libx264.so.163"
        "libx265.so.199"
        "libopenh264.so.7"
        "libopus.so.0"
        "libwebp.so.7"
        "libwebpmux.so.3"
        "libcairo.so.2"
        "libdrm.so.2"
    )
    for helper in "${helper_libs[@]}"; do
        local copied_path=""
        if copied_path=$(copy_system_library "${helper}" "${gst_lib_dest}"); then
            "${LINUXDEPLOY}" --appdir "${APPDIR}" --library "${copied_path}" >/dev/null
        else
            echo "warning: Optional multimedia dependency ${helper} not found; continuing." >&2
        fi
    done

    local qt_mediaservice_src=""
    for base in "${LIB_SEARCH_PATHS[@]}"; do
        if [[ -d "${base}/qt6/plugins/mediaservice" ]]; then
            qt_mediaservice_src="${base}/qt6/plugins/mediaservice"
            break
        fi
    done
    if [[ -n "${qt_mediaservice_src}" ]]; then
        mkdir -p "${qt_mediaservice_dest}"
        cp -a "${qt_mediaservice_src}/." "${qt_mediaservice_dest}/"
    else
        echo "warning: Qt mediaservice plugins not found on host system." >&2
    fi

    local gst_scanner_path=""
    if command -v gst-plugin-scanner >/dev/null 2>&1; then
        gst_scanner_path="$(command -v gst-plugin-scanner)"
    else
        local -a scanner_search_roots=()
        for base in "${LIB_SEARCH_PATHS[@]}"; do
            if [[ -d "${base}" ]]; then
                scanner_search_roots+=("${base}")
            fi
        done
        if [[ -d "/usr/libexec" ]]; then
            scanner_search_roots+=("/usr/libexec")
        fi
        if ((${#scanner_search_roots[@]} > 0)); then
            gst_scanner_path="$(find "${scanner_search_roots[@]}" -maxdepth 3 -type f -name 'gst-plugin-scanner' 2>/dev/null | head -n1 || true)"
        else
            gst_scanner_path=""
        fi
    fi
    if [[ -n "${gst_scanner_path}" ]]; then
        local gst_scanner_dest="${APPDIR}/usr/libexec/gstreamer-1.0"
        mkdir -p "${gst_scanner_dest}"
        cp -a "${gst_scanner_path}" "${gst_scanner_dest}/"
        chmod +x "${gst_scanner_dest}/gst-plugin-scanner"
        "${LINUXDEPLOY}" --appdir "${APPDIR}" --executable "${gst_scanner_dest}/gst-plugin-scanner" >/dev/null
    else
        echo "warning: gst-plugin-scanner not found; GStreamer plugin discovery may fail." >&2
    fi

    if [[ -d "${qt_mediaservice_dest}" ]]; then
        while IFS= read -r lib; do
            [[ -f "${lib}" ]] || continue
            "${LINUXDEPLOY}" --appdir "${APPDIR}" --library "${lib}" >/dev/null
        done < <(find "${qt_mediaservice_dest}" -type f -name '*.so*')
    fi

    while IFS= read -r lib; do
        [[ -f "${lib}" ]] || continue
        "${LINUXDEPLOY}" --appdir "${APPDIR}" --library "${lib}" >/dev/null
    done < <(find "${gst_plugin_dest}" -type f -name '*.so*')
}

generate_apprun() {
    cat > "${APPDIR}/AppRun" <<'EOF'
#!/usr/bin/env bash
set -euo pipefail

APPDIR="${APPDIR:-$(dirname "$(readlink -f "$0")")}"

export LD_LIBRARY_PATH="${APPDIR}/usr/lib:${APPDIR}/usr/lib/gstreamer-1.0:${LD_LIBRARY_PATH:-}"
export QT_PLUGIN_PATH="${APPDIR}/usr/plugins:${APPDIR}/usr/lib/qt6/plugins:${QT_PLUGIN_PATH:-}"
export QT_QPA_PLATFORM_PLUGIN_PATH="${APPDIR}/usr/lib/qt6/plugins/platforms"
export QML_IMPORT_PATH="${APPDIR}/usr/qml:${QML_IMPORT_PATH:-}"
export GST_PLUGIN_SYSTEM_PATH="${APPDIR}/usr/lib/gstreamer-1.0"
export GST_PLUGIN_SYSTEM_PATH_1_0="${APPDIR}/usr/lib/gstreamer-1.0"
if [[ -x "${APPDIR}/usr/libexec/gstreamer-1.0/gst-plugin-scanner" ]]; then
    export GST_PLUGIN_SCANNER="${APPDIR}/usr/libexec/gstreamer-1.0/gst-plugin-scanner"
fi

exec "${APPDIR}/usr/bin/obsbot-gui" "$@"
EOF
    chmod +x "${APPDIR}/AppRun"
}

pushd "${BUILD_DIR}" >/dev/null
rm -f obsbot-control-linux-*.AppImage

"${LINUXDEPLOY_QT}" --appdir "${APPDIR}" "${QT_EXTRA_ARGS[@]}"
bundle_sdk_runtime
bundle_gstreamer_assets
generate_apprun
"${LINUXDEPLOY}" --appdir "${APPDIR}" --deploy-deps-only --executable "${APPDIR}/usr/bin/obsbot-gui" >/dev/null
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
