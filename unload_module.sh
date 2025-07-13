#!/bin/bash

# Proper Module Unloader
# This script unhides the module first, then unloads it

set -e

echo "=== Proper Module Unloader ==="

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

print_info "Starting proper module unload process..."

# Step 1: Check if module is loaded
print_status "1. Checking if module is loaded..."
if cat /proc/modules | grep -q "kernel.*20480"; then
    print_status "Module found (disguised as 'kernel')"
elif lsmod | grep -q "logkey"; then
    print_status "Module found (visible as 'logkey')"
else
    print_warning "Module not found in loaded modules"
fi

# Step 2: Stop user-space client if running
print_status "2. Stopping user-space client..."
pkill -f "logkey_client" 2>/dev/null || true
sleep 1

# Step 3: Unhide module using /proc interface
print_status "3. Unhiding module..."
if [ -f "/proc/unhide_logkey" ]; then
    echo "unhide" > /proc/unhide_logkey
    print_status "Unhide command sent to module"
    
    # Wait a moment for unhide to complete
    sleep 1
    
    # Check if module is now visible
    if lsmod | grep -q "logkey"; then
        print_status "Module successfully unhidden"
    else
        print_warning "Module still not visible in lsmod"
    fi
else
    print_warning "/proc/unhide_logkey not found - module may not support unhide"
fi

# Step 4: Try to unload module
print_status "4. Unloading module..."
if lsmod | grep -q "logkey"; then
    rmmod logkey
    if [ $? -eq 0 ]; then
        print_status "Module unloaded successfully"
    else
        print_error "Failed to unload module"
        print_info "You may need to force unload or reboot"
    fi
else
    print_warning "Module not found in lsmod - trying alternative methods..."
    
    # Try to unload by major number
    if [ -e "/dev/keylogger" ]; then
        major=$(stat -c %t /dev/keylogger)
        print_info "Trying to unload by major number: $major"
        # This is a fallback method
    fi
    
    print_error "Cannot unload module - it may be hidden too deeply"
    print_info "You may need to reboot to remove the module"
fi

# Step 5: Clean up device file
print_status "5. Cleaning up device file..."
if [ -e "/dev/keylogger" ]; then
    rm -f /dev/keylogger
    print_status "Device file removed"
else
    print_status "Device file not found"
fi

# Step 6: Clean up symlinks
print_status "6. Cleaning up symlinks..."
for symlink in /dev/random_backup /dev/urandom_backup /dev/null_backup; do
    if [ -L "$symlink" ]; then
        rm -f "$symlink"
        print_status "Removed symlink: $symlink"
    fi
done

# Step 7: Clean up decoy files
print_status "7. Cleaning up decoy files..."
for decoy in /tmp/.system_config /tmp/.kernel_config /tmp/.driver_config; do
    if [ -f "$decoy" ]; then
        rm -f "$decoy"
        print_status "Removed decoy file: $decoy"
    fi
done

# Step 8: Clean up log files
print_status "8. Cleaning up log files..."
if [ -f "/tmp/keylog.txt" ]; then
    rm -f /tmp/keylog.txt
    print_status "Removed log file: /tmp/keylog.txt"
fi

# Step 9: Clean build artifacts
print_status "9. Cleaning build artifacts..."
cd kernel && make clean
cd ../user && make clean
print_status "Build artifacts cleaned"

# Final verification
echo ""
print_info "=== Final Verification ==="

# Check if module is still loaded
if lsmod | grep -q "logkey"; then
    print_error "Module still loaded"
elif cat /proc/modules | grep -q "kernel.*20480"; then
    print_error "Module still loaded (disguised)"
else
    print_status "Module successfully unloaded"
fi

# Check if device file is gone
if [ -e "/dev/keylogger" ]; then
    print_error "Device file still exists"
else
    print_status "Device file removed"
fi

# Check if proc file is gone
if [ -f "/proc/unhide_logkey" ]; then
    print_error "Proc file still exists"
else
    print_status "Proc file removed"
fi

print_status "Module unload process completed!"
print_info "If module is still loaded, you may need to reboot the system" 