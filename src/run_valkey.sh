#!/bin/bash

# Build modules
cd modules
make

# Change back to src directory
cd ..

# Run valkey-server in background
./valkey-server valkey.conf 
