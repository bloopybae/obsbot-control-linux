#!/usr/bin/env bash
set -euo pipefail

ACTION="${1:-}"
SERVICE_SOURCE="${2:-}"
MODPROBE_SOURCE="${3:-}"

MODPROBE_BIN="$(command -v modprobe || true)"
if [[ -z "${MODPROBE_BIN}" ]]; then
    echo "modprobe not found on PATH" >&2
    exit 1
fi

if [[ -z "$ACTION" ]]; then
    echo "Usage: $0 <action> <service-source> <modprobe-source>" >&2
    exit 1
fi

SERVICE_TARGET="/etc/systemd/system/obsbot-virtual-camera.service"
MODPROBE_TARGET="/etc/modprobe.d/obsbot-virtual-camera.conf"

install_files() {
    install -Dm644 "$SERVICE_SOURCE" "$SERVICE_TARGET"
    install -Dm644 "$MODPROBE_SOURCE" "$MODPROBE_TARGET"
    systemctl daemon-reload
}

enable_service() {
    systemctl enable --now obsbot-virtual-camera.service
}

disable_service() {
    systemctl disable --now obsbot-virtual-camera.service || true
}

remove_files() {
    disable_service
    rm -f "$SERVICE_TARGET"
    rm -f "$MODPROBE_TARGET"
    systemctl daemon-reload
}

case "$ACTION" in
    install)
        install_files
        ;;
    enable)
        enable_service
        ;;
    disable)
        disable_service
        ;;
    remove)
        remove_files
        ;;
    load-once)
        "${MODPROBE_BIN}" v4l2loopback video_nr=42 card_label="OBSBOT Virtual Camera" exclusive_caps=1
        ;;
    unload)
        "${MODPROBE_BIN}" -r v4l2loopback || true
        ;;
    *)
        echo "Unknown action: $ACTION" >&2
        exit 1
        ;;
esac
