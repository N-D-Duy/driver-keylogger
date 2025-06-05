#!/bin/bash


set -e

KERNEL_DIR=kernel
USER_DIR=user
DEV_NAME=logkey

echo "[*] Building kernel module..."
make -C $KERNEL_DIR

echo "[*] Building user program..."
make -C $USER_DIR

echo "[*] Inserting module..."
sudo insmod $KERNEL_DIR/logkey.ko

MAJOR=$(sudo dmesg | grep "$DEV_NAME loaded" | tail -1 | grep -oP 'Major: \K[0-9]+')
echo "[*] Detected major number: $MAJOR"

echo "[*] Creating /dev/$DEV_NAME..."
sudo mknod /dev/$DEV_NAME c $MAJOR 0
sudo chmod 666 /dev/$DEV_NAME

echo "[*] Running user program..."
$USER_DIR/logkey_client

echo "[*] Cleaning up..."
sudo rmmod logkey
sudo rm -f /dev/$DEV_NAME

echo "[*] Clean project"
make clean