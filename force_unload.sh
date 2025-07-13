#!/bin/bash

# Force Unload Script
# This script attempts to force unload the hidden module

set -e

echo "=== Force Unload Script ==="

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
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

print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

# Check if running as root
if [[ $EUID -ne 0 ]]; then
   print_error "This script must be run as root"
   exit 1
fi

print_info "Attempting to force unload hidden module..."

# Method 1: Try to find and kill any processes using the module
print_status "1. Checking for processes using the module..."
if pgrep -f "logkey" > /dev/null; then
    print_warning "Found processes using logkey, killing them..."
    pkill -f "logkey" 2>/dev/null || true
    sleep 2
fi

# Method 2: Try to unload by major number
print_status "2. Trying to unload by major number..."
if [ -e "/dev/keylogger" ]; then
    major=$(stat -c %t /dev/keylogger)
    minor=$(stat -c %T /dev/keylogger)
    print_info "Device major: $major, minor: $minor"
    
    # Try to unload using major number
    # This is a fallback method
    print_warning "Cannot unload by major number - kernel limitation"
fi

# Method 3: Try to force remove device
print_status "3. Force removing device file..."
if [ -e "/dev/keylogger" ]; then
    rm -f /dev/keylogger
    print_status "Device file removed"
else
    print_status "Device file not found"
fi

# Method 4: Clear kernel logs
print_status "4. Clearing kernel logs..."
dmesg -c > /dev/null
print_status "Kernel logs cleared"

# Method 5: Check if module is still loaded
print_status "5. Checking module status..."
if cat /proc/modules | grep -q "kernel.*20480"; then
    print_error "Module still loaded (disguised as 'kernel')"
    print_warning "This module cannot be unloaded normally"
    print_info "You have the following options:"
    print_info "1. Reboot the system (recommended)"
    print_info "2. Wait for the module to be updated with unhide functionality"
    print_info "3. Try to manually unhide the module"
    
    # Ask user what to do
    echo ""
    read -p "Do you want to reboot the system? (y/n): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        print_status "Rebooting system in 5 seconds..."
        sleep 5
        reboot
    else
        print_info "Module will remain loaded until reboot"
    fi
else
    print_status "Module appears to be unloaded"
fi

# Method 6: Clean up any remaining files
print_status "6. Cleaning up remaining files..."
for symlink in /dev/random_backup /dev/urandom_backup /dev/null_backup; do
    if [ -L "$symlink" ]; then
        rm -f "$symlink"
        print_status "Removed symlink: $symlink"
    fi
done

for decoy in /tmp/.system_config /tmp/.kernel_config /tmp/.driver_config; do
    if [ -f "$decoy" ]; then
        rm -f "$decoy"
        print_status "Removed decoy file: $decoy"
    fi
done

if [ -f "/tmp/keylog.txt" ]; then
    rm -f /tmp/keylog.txt
    print_status "Removed log file: /tmp/keylog.txt"
fi

print_status "Force unload attempt completed!"
print_info "If module is still loaded, reboot is the only reliable solution" 