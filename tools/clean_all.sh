#!/bin/bash

echo "Cleaning build outputs..."

# Remove everything in build/ except README.md
shopt -s extglob
cd build || exit 1
rm -rf !(README.md)
cd ..

# Remove generated device key
rm -f src/global/device_key.generated.h

echo "Clean complete."
