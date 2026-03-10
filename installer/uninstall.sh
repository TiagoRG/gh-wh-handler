#!/bin/bash

ask_user_permission() {
    local PROMPT_TEXT=$1
    printf "${PROMPT_TEXT} [Y/n]: "

    read -n 1 -s -r USER_INPUT

    if [[ -z "$USER_INPUT" ]] || [[ "$USER_INPUT" =~ ^[Yy]$ ]]; then
        echo "Y"
        return 0
    else
        echo "n"
        return 1
    fi
}

if [ "$(id -u)" -ne 0 ]; then
    echo "Error: This script must be run as root (UID 0). Current UID: $(id -u)"
    exit 1
fi

ARCH=$(uname -m)
echo "Uninstalling gh-wh-handler [${ARCH}]..."

echo "Stopping and disabling service..."
systemctl stop gh-wh-handler 2>/dev/null
systemctl disable gh-wh-handler 2>/dev/null

echo "Removing service file..."
rm -f "/etc/systemd/system/gh-wh-handler.service"

echo "Reloading systemd..."
systemctl daemon-reload

echo "Removing binaries..."
rm -f "/usr/bin/gh-wh-handler"
rm -f "/usr/bin/gh-wh-handler.${ARCH}"

if ask_user_permission "Do you want to remove the configuration file?"; then
    echo "Removing /etc/gh-wh-handler..."
    rm -rf "/etc/gh-wh-handler"
else
    echo "Skipping configuration removal."
fi

if ask_user_permission "Do you want to remove the logs?"; then
    echo "Removing /var/log/gh-wh-handler..."
    rm -rf "/var/log/gh-wh-handler"
else
    echo "Skipping log removal."
fi

echo "Uninstallation complete."
