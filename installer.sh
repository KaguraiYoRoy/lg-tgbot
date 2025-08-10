#!/usr/bin/env bash
set -e

if [ "$(id -u)" -ne 0 ]; then
  echo "Error: This script must be run as root."
  exit 1
fi


REPO="KaguraiYoRoy/lg-tgbot"
INSTALL_DIR="/opt/lg-tgbot"
USER_NAME="lg-tgbot"

usage() {
    echo "Usage: $0 {install|uninstall} {master|agent}"
    exit 1
}

check_arch() {
    local arch
    arch=$(uname -m)
    case "$arch" in
        x86_64) ARCH="x86_64" ;;
        aarch64) ARCH="arm64" ;;
        *) echo "Unsupported architect: $arch"; exit 1 ;;
    esac
}

get_latest_release_url() {
    local component=$1
    if [[ "$component" == "master" ]]; then
        FILE="lg-tgbot_${ARCH}.tar.gz"
    else
        FILE="lg-tgbot-agent_${ARCH}.tar.gz"
    fi
    
    curl -s https://api.github.com/repos/$REPO/releases/latest \
        | grep "browser_download_url" \
        | grep "$FILE" \
        | cut -d '"' -f 4
}

create_user() {
    if ! id "$USER_NAME" &>/dev/null; then
        useradd --system --no-create-home --shell /usr/sbin/nologin "$USER_NAME"
    fi
}

install_component() {
    local component=$1
    check_arch
    create_user

    mkdir -p "$INSTALL_DIR/$component"
    URL=$(get_latest_release_url "$component")
    if [[ -z "$URL" ]]; then
        echo "Failed to get download url"
        exit 1
    fi

    TMP_FILE=$(mktemp)
    echo "Download: $URL"
    curl -L "$URL" -o "$TMP_FILE"
    tar -xzf "$TMP_FILE" -C "$INSTALL_DIR/$component"
    rm "$TMP_FILE"

    chown -R "$USER_NAME":"$USER_NAME" "$INSTALL_DIR"
    chmod -R 755 "$INSTALL_DIR"

    if [[ "$component" == "master" ]]; then
        SERVICE_FILE="/etc/systemd/system/lg-tgbot-master.service"
        cat > "$SERVICE_FILE" <<EOF
[Unit]
Description = Looking-Glass on Telegram Bot
After = network.target

[Service]
Type = idle
User = $USER_NAME
Restart = always
RestartSec = 30s
ExecStart = $INSTALL_DIR/master/lg-tgbot
WorkingDirectory=$INSTALL_DIR/master

[Install]
WantedBy = multi-user.target
EOF
    else
        SERVICE_FILE="/etc/systemd/system/lg-tgbot-agent.service"
        cat > "$SERVICE_FILE" <<EOF
[Unit]
Description = Looking-Glass agent on Telegram Bot
After = network.target

[Service]
Type = idle
User = $USER_NAME
Restart = always
RestartSec = 30s
ExecStart = $INSTALL_DIR/agent/lg-tgbot-agent
WorkingDirectory=$INSTALL_DIR/agent

[Install]
WantedBy = multi-user.target
EOF
    fi

    systemctl daemon-reload
    systemctl enable --now "$(basename "$SERVICE_FILE")"
    echo "$component install success"
    echo "You need to create config file manually at /opt/lg-tgbot/$component/config.json"
}

uninstall_component() {
    local component=$1
    if [[ "$component" == "master" ]]; then
        SERVICE_FILE="/etc/systemd/system/lg-tgbot-master.service"
    else
        SERVICE_FILE="/etc/systemd/system/lg-tgbot-agent.service"
    fi

    systemctl stop "$(basename "$SERVICE_FILE")" || true
    systemctl disable "$(basename "$SERVICE_FILE")" || true
    rm -f "$SERVICE_FILE"
    rm -rf "$INSTALL_DIR/$component"
    systemctl daemon-reload

    if [[ ! -d "$INSTALL_DIR/master" && ! -d "$INSTALL_DIR/agent" ]]; then
        userdel "$USER_NAME" || true
        rm -rf "$INSTALL_DIR"
    fi

    echo "$component removed"
}

main() {
    if [[ $# -ne 2 ]]; then
        usage
    fi

    ACTION=$1
    COMPONENT=$2

    if [[ "$COMPONENT" != "master" && "$COMPONENT" != "agent" ]]; then
        usage
    fi

    case "$ACTION" in
        install) install_component "$COMPONENT" ;;
        uninstall) uninstall_component "$COMPONENT" ;;
        *) usage ;;
    esac
}

main "$@"
