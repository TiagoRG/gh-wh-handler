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

# Change to the temporary directory
cd /tmp

# Download the latest version of the package for the system architecture
# Exit if the download fails
echo "Downloading gh-wh-handler..."
curl -fsSL https://cdn.tiagorg.pt/gh-wh-handler/gh-wh-handler.${ARCH}.latest.tar.gz -o gh-wh-handler.tar.gz || { echo "Download failed."; exit 1; }

# Extract the package
echo "Extracting gh-wh-handler..."
tar -xzf gh-wh-handler.tar.gz || { echo "Extraction failed."; exit 1; }

# Change to the extracted directory
cd gh-wh-handler

# Install the package
echo "Installing gh-wh-handler..."

# Create service directory
echo "Creating service directory..."
mkdir -p /services/gh-wh-handler
mkdir -p /services/gh-wh-handler/logs

# Copy the binary and configuration file to the service directory
echo "Copying files..."
cp "gh-wh-handler.${ARCH}" /services/gh-wh-handler/
cp "config.json" /services/gh-wh-handler/

# Create a symbolic link to the binary in /usr/bin
echo "Creating symbolic link..."
ln -sf /services/gh-wh-handler/gh-wh-handler.${ARCH} /usr/bin/gh-wh-handler

# Copy the service file to the systemd directory
echo "Copying service file..."
cp "gh-wh-handler.service" /etc/systemd/system/

# Reload systemd
echo "Reloading systemd..."
systemctl daemon-reload

# Enable and start the service
echo "Enabling and starting service..."
systemctl enable gh-wh-handler
systemctl start gh-wh-handler

# Clean up
echo "Cleaning up..."
cd /tmp
rm -rf gh-wh-handler
rm gh-wh-handler.tar.gz

# Change back to the original directory
cd $CUR_DIR
echo "Installation complete."
