#!/bin/bash

# Build script for Reverb Plugin
set -e

echo "🎵 Building Reverb Plugin..."

# Create build directory
mkdir -p build
cd build

# Configure with CMake
echo "📦 Configuring project..."
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build the plugin
echo "🔨 Building plugin..."
cmake --build . --config Release

echo "✅ Build completed!"
echo ""
echo "Plugin files created:"
find . -name "*ReverbPlugin*" -type f | head -10

echo ""
echo "🚀 To test the plugin:"
echo "1. Standalone app: Look for ReverbPlugin executable"
echo "2. VST3: Copy .vst3 file to your DAW's plugin folder"
echo "3. AU (macOS): Copy .component to ~/Library/Audio/Plug-Ins/Components/"