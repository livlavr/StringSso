#!/bin/bash

set -e

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

BUILD_DIR="build"
TARGET="sso_tests"

if [ ! -d "$BUILD_DIR" ]; then
    echo "Creating build directory..."
    mkdir "$BUILD_DIR"
fi

echo -e "${GREEN}==> Configuring project...${NC}"
cmake -S . -B "$BUILD_DIR" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

echo -e "${GREEN}==> Building target: $TARGET...${NC}"
cmake --build "$BUILD_DIR" --target "$TARGET" -j $(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 1)

echo -e "${GREEN}==> Running tests...${NC}"
cd "$BUILD_DIR"

./tests/sso/"$TARGET"

echo -e "${GREEN}==> All steps completed successfully!${NC}"
