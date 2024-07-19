#!/bin/sh

# Check if the script is being run as root
if [ "$(id -u)" -ne 0 ]; then
    echo "This script must be run as root."
    exit 1
fi

# Save the current directory
CUR_DIR=$(pwd)

# Get system architecture
ARCH=$(uname -m)

echo "Uninstalling gh-wh-handler..."

# Stop and disable the service
echo "Stopping and disabling service..."
systemctl stop gh-wh-handler
systemctl disable gh-wh-handler

# Remove the service file
echo "Removing service file..."
rm /etc/systemd/system/gh-wh-handler.service

# Reload systemd
echo "Reloading systemd..."
systemctl daemon-reload

# Remove the symbolic link
echo "Removing symbolic link..."
rm /usr/bin/gh-wh-handler

# Remove the logs directory and binary
echo "Removing files..."
rm -rf /services/gh-wh-handler/logs
rm -f /services/gh-wh-handler/gh-wh-handler.${ARCH}

# Change back to the original directory
cd $CUR_DIR
echo "Uninstallation complete."

