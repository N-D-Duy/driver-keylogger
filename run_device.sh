#!/bin/bash

set -e

KERNEL_DIR=kernel
USER_DIR=user

## clean file, module, device node
# sudo rm -f /dev/keylogger
sudo rmmod logkey

echo "[*] Building kernel module (device version)..."
make -C $KERNEL_DIR

echo "[*] Building user space (device version)..."
make -C $USER_DIR

echo "[*] Inserting device module..."
sudo insmod $KERNEL_DIR/logkey.ko

echo "[*] Creating device node..."
sudo rm -f /dev/keylogger
sudo mknod /dev/keylogger c $(cat /proc/devices | grep keylogger | awk '{print $1}') 0
sudo chmod 666 /dev/keylogger

echo "[*] Module inserted. Device created at /dev/keylogger"
echo "[*] Running user space client..."

$USER_DIR/logkey_client

read -p "[*] Press Enter to remove module..."

echo "[*] Removing module..."
sudo rmmod logkey
sudo rm -f /dev/keylogger 