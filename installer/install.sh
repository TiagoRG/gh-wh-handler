#!/bin/sh

if [ "$(id -u)" -ne 0 ]; then
    echo "Error: This script must be run as root (UID 0). Current UID: $(id -u)"
    exit 1
fi

CUR_DIR=$(pwd)

ARCH=$(uname -m)

cd /tmp

echo "Downloading gh-wh-handler..."
curl -fsSL https://cdn.tiagorg.pt/gh-wh-handler/gh-wh-handler.${ARCH}.latest.tar.gz -o gh-wh-handler.tar.gz || { echo "Download failed."; exit 1; }

echo "Extracting gh-wh-handler..."
tar -xzf gh-wh-handler.tar.gz || { echo "Extraction failed."; exit 1; }

cd gh-wh-handler

echo "Installing gh-wh-handler..."

echo "Creating directories..."
mkdir -p /etc/gh-wh-handler
mkdir -p /var/log/gh-wh-handler

echo "Copying files..."
cp "gh-wh-handler.${ARCH}" /usr/bin/
ln -s /usr/bin/gh-wh-handler.${ARCH} /usr/bin/gh-wh-handler
cp "config.json" /etc/gh-wh-handler/

echo "Copying service file..."
cp "gh-wh-handler.service" /etc/systemd/system/

echo "Reloading systemd..."
systemctl daemon-reload

echo "Enabling and starting service..."
systemctl enable gh-wh-handler
systemctl start gh-wh-handler

echo "Cleaning up..."
cd /tmp
rm -rf gh-wh-handler
rm gh-wh-handler.tar.gz

cd $CUR_DIR
echo "Installation complete."
