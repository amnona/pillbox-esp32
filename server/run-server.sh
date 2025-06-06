#!/bin/bash

echo "Starting Pillbox ESP32 Server"
conda activate pillbox-server
cd git/pillbox-esp32/server
./server.py
echo "Running server"
