#!/bin/bash

# Stealth Detection Checker
# This script checks if the keylogger module is properly hidden

echo "=== Stealth Detection Checker ==="

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if running as root
if [[ $EUID -ne 0 ]]; then
   print_error "This script should be run as root for complete detection"
   print_warning "Some checks may not work without root privileges"
fi

echo ""
print_info "Checking for keylogger module detection..."

# 1. Check /proc/modules
echo ""
print_info "1. Checking /proc/modules:"
if grep -q "logkey" /proc/modules; then
    print_error "Module 'logkey' found in /proc/modules"
    grep "logkey" /proc/modules
else
    print_success "Module 'logkey' not found in /proc/modules"
fi

# Check for disguised name
if grep -q "kernel" /proc/modules; then
    print_warning "Suspicious 'kernel' entry found in /proc/modules"
    grep "kernel" /proc/modules
fi

# 2. Check lsmod
echo ""
print_info "2. Checking lsmod output:"
if lsmod | grep -q "logkey"; then
    print_error "Module 'logkey' found in lsmod"
    lsmod | grep "logkey"
else
    print_success "Module 'logkey' not found in lsmod"
fi

# 3. Check /sys/module
echo ""
print_info "3. Checking /sys/module:"
if [ -d "/sys/module/logkey" ]; then
    print_error "Module directory found: /sys/module/logkey"
    ls -la /sys/module/logkey/
else
    print_success "Module directory not found: /sys/module/logkey"
fi

# 4. Check device file
echo ""
print_info "4. Checking device file:"
if [ -e "/dev/keylogger" ]; then
    print_warning "Device file found: /dev/keylogger"
    ls -la /dev/keylogger
    
    # Check permissions
    perms=$(stat -c %a /dev/keylogger)
    if [ "$perms" = "600" ]; then
        print_success "Device has secure permissions (600)"
    else
        print_error "Device has insecure permissions: $perms"
    fi
else
    print_success "Device file not found: /dev/keylogger"
fi

# 5. Check /proc/devices
echo ""
print_info "5. Checking /proc/devices:"
if grep -q "keylogger" /proc/devices; then
    print_error "Device 'keylogger' found in /proc/devices"
    grep "keylogger" /proc/devices
else
    print_success "Device 'keylogger' not found in /proc/devices"
fi

# 6. Check kernel symbols
echo ""
print_info "6. Checking kernel symbols:"
if [ -f "/proc/kallsyms" ]; then
    if grep -q "logkey" /proc/kallsyms; then
        print_error "Module symbols found in /proc/kallsyms"
        grep "logkey" /proc/kallsyms | head -5
    else
        print_success "Module symbols not found in /proc/kallsyms"
    fi
else
    print_warning "Cannot access /proc/kallsyms (requires root)"
fi

# 7. Check loaded kernel modules
echo ""
print_info "7. Checking loaded kernel modules:"
loaded_modules=$(cat /proc/modules | awk '{print $1}')
suspicious_modules=("keylogger" "logger" "key" "log" "spy" "monitor")

for module in "${suspicious_modules[@]}"; do
    if echo "$loaded_modules" | grep -q "$module"; then
        print_warning "Suspicious module found: $module"
        cat /proc/modules | grep "$module"
    fi
done

# 8. Check for suspicious processes
echo ""
print_info "8. Checking for suspicious processes:"
suspicious_processes=("logkey" "keylogger" "logger" "spy" "monitor")

for proc in "${suspicious_processes[@]}"; do
    if pgrep -f "$proc" > /dev/null; then
        print_warning "Suspicious process found: $proc"
        ps aux | grep "$proc" | grep -v grep
    fi
done

# 9. Check for suspicious files
echo ""
print_info "9. Checking for suspicious files:"
suspicious_files=("/dev/keylogger" "/proc/keylogger" "/sys/keylogger")

for file in "${suspicious_files[@]}"; do
    if [ -e "$file" ]; then
        print_warning "Suspicious file found: $file"
        ls -la "$file"
    fi
done

# 10. Check for network connections
echo ""
print_info "10. Checking for suspicious network connections:"
if command -v netstat >/dev/null 2>&1; then
    if netstat -tuln 2>/dev/null | grep -q "65432"; then
        print_warning "Suspicious network connection on port 65432"
        netstat -tuln 2>/dev/null | grep "65432"
    fi
elif command -v ss >/dev/null 2>&1; then
    if ss -tuln 2>/dev/null | grep -q "65432"; then
        print_warning "Suspicious network connection on port 65432"
        ss -tuln 2>/dev/null | grep "65432"
    fi
fi

# Summary
echo ""
print_info "=== Detection Summary ==="
echo "This script checks for common keylogger detection methods."
echo "If you see any [ERROR] or [WARNING] messages above,"
echo "the keylogger may be detectable by security tools."
echo ""
print_info "For maximum stealth, ensure all checks show [SUCCESS]" 