#!/bin/bash

# Build script for cpp-switchbot

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}Building cpp-switchbot...${NC}"

# Check if build directory exists
if [ -d "build" ]; then
    echo -e "${YELLOW}Build directory exists. Cleaning...${NC}"
    rm -rf build
fi

# Create build directory
mkdir build
cd build

# Run CMake
echo -e "${GREEN}Running CMake...${NC}"
cmake ..

# Build
echo -e "${GREEN}Compiling...${NC}"
make -j$(nproc)

echo -e "${GREEN}Build complete!${NC}"
echo -e "${GREEN}Example executable: ./build/switchbot_example${NC}"
