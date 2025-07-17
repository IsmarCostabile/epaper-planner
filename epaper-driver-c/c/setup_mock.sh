#!/bin/bash

# Setup script for EPD 7in3f Mock Development Environment
# This script sets up the mock environment for developing without hardware

echo "EPD 7in3f Mock Development Environment Setup"
echo "=============================================="

# Check if we're on macOS
if [[ "$OSTYPE" == "darwin"* ]]; then
    echo "Detected macOS"
    
    # Check if Homebrew is installed
    if ! command -v brew &> /dev/null; then
        echo "Homebrew not found. Installing Homebrew..."
        /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
    fi
    
    # Install SDL2
    echo "Installing SDL2..."
    brew install sdl2
    
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    echo "Detected Linux"
    
    # Check the distribution
    if command -v apt-get &> /dev/null; then
        echo "Installing SDL2 via apt-get..."
        sudo apt-get update
        sudo apt-get install -y libsdl2-dev
    elif command -v yum &> /dev/null; then
        echo "Installing SDL2 via yum..."
        sudo yum install -y SDL2-devel
    elif command -v dnf &> /dev/null; then
        echo "Installing SDL2 via dnf..."
        sudo dnf install -y SDL2-devel
    else
        echo "Could not determine package manager. Please install SDL2 manually."
        exit 1
    fi
    
else
    echo "Unsupported OS: $OSTYPE"
    echo "Please install SDL2 manually and try again."
    exit 1
fi

# Create bin directory if it doesn't exist
echo "Creating bin directory..."
mkdir -p bin

# Build the mock version
echo "Building EPD 7in3f mock version..."
make clean
make EPD=epd7in3f_mock MOCK

if [ $? -eq 0 ]; then
    echo ""
    echo "✅ Build successful!"
    echo ""
    echo "To run the mock EPD display:"
    echo "  ./epd"
    echo ""
    echo "The mock display will:"
    echo "  - Show a live SDL2 window with the EPD content"
    echo "  - Log all GPIO operations to /tmp/gpio_mock.log"
    echo "  - Log all SPI operations to /tmp/spi_mock.log"
    echo "  - Save screenshots to /tmp/epd_screenshot_*.bmp"
    echo ""
    echo "Press ESC or close the window to exit."
    echo ""
else
    echo "❌ Build failed!"
    echo ""
    echo "Common issues:"
    echo "1. SDL2 not properly installed"
    echo "2. Missing development headers"
    echo "3. Compiler errors"
    echo ""
    echo "Check the build output above for specific errors."
    exit 1
fi
