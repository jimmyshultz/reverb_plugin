# Build Instructions

This document provides instructions for building the Reverb Plugin from source.

## Prerequisites

### Required Tools
- **CMake** 3.15 or later
- **C++ Compiler** with C++17 support:
  - Windows: Visual Studio 2019 or later / MinGW
  - macOS: Xcode 11 or later
  - Linux: GCC 9 or later / Clang 9 or later

### JUCE Framework
- **JUCE** 7.0 or later
- Download from: https://juce.com/get-juce/

## Platform-Specific Setup

### Windows
1. Install Visual Studio 2019 or later with C++ development tools
2. Install CMake from https://cmake.org/download/
3. Clone or download JUCE framework
4. Set environment variable `JUCE_PATH` to JUCE installation directory

### macOS
1. Install Xcode from the App Store
2. Install Xcode command line tools: `xcode-select --install`
3. Install CMake: `brew install cmake`
4. Clone or download JUCE framework
5. Set environment variable `JUCE_PATH` to JUCE installation directory

### Linux (Ubuntu/Debian)
```bash
sudo apt update
sudo apt install build-essential cmake git
sudo apt install libasound2-dev libjack-jackd2-dev \
    ladspa-sdk \
    libcurl4-openssl-dev \
    libfreetype6-dev \
    libx11-dev libxcomposite-dev libxcursor-dev \
    libxinerama-dev libxrandr-dev libxrender-dev \
    libwebkit2gtk-4.0-dev \
    libglu1-mesa-dev mesa-common-dev
```

## Building the Plugin

### Using CMake (Recommended)

1. **Clone the repository:**
```bash
git clone https://github.com/yourcompany/reverb-plugin.git
cd reverb-plugin
```

2. **Create build directory:**
```bash
mkdir build
cd build
```

3. **Configure the project:**
```bash
# Point to your JUCE installation
cmake .. -DJUCE_PATH=/path/to/JUCE

# Or if JUCE is installed system-wide
cmake ..
```

4. **Build the plugin:**
```bash
# Build all targets
cmake --build .

# Or build specific configuration
cmake --build . --config Release
```

### Using Projucer (Alternative)

1. **Open Projucer** (part of JUCE distribution)
2. **Open the .jucer file:** `ReverbPlugin.jucer`
3. **Set up exporters** for your target platforms
4. **Save and open** in your IDE (Xcode, Visual Studio, etc.)
5. **Build** using your IDE's build system

## Build Targets

The build system creates the following plugin formats:

### VST3
- **Windows:** `ReverbPlugin.vst3`
- **macOS:** `ReverbPlugin.vst3`
- **Linux:** `ReverbPlugin.vst3`

### Audio Units (macOS only)
- **macOS:** `ReverbPlugin.component`

### Standalone Application
- **All platforms:** `ReverbPlugin` (executable)

## Installation

### Automatic Installation
After building, the plugins are automatically copied to standard locations:

- **Windows VST3:** `%ProgramFiles%\Common Files\VST3\`
- **macOS VST3:** `/Library/Audio/Plug-Ins/VST3/`
- **macOS AU:** `/Library/Audio/Plug-Ins/Components/`
- **Linux VST3:** `~/.vst3/`

### Manual Installation
Copy the built plugin files to your DAW's plugin directory.

## Build Options

### Configuration Options
```bash
# Release build (optimized)
cmake --build . --config Release

# Debug build (with debug symbols)
cmake --build . --config Debug

# Specify plugin formats
cmake .. -DFORMATS="VST3;AU;Standalone"

# Enable/disable specific features
cmake .. -DENABLE_CONVOLUTION=ON -DENABLE_MODULATION=ON
```

### Cross-Platform Building

#### Windows for macOS (using osxcross)
```bash
cmake .. -DCMAKE_TOOLCHAIN_FILE=cmake/osxcross.cmake
```

#### Linux for Windows (using MinGW)
```bash
cmake .. -DCMAKE_TOOLCHAIN_FILE=cmake/mingw.cmake
```

## Troubleshooting

### Common Issues

**JUCE not found:**
- Ensure JUCE_PATH environment variable is set
- Or specify manually: `-DJUCE_PATH=/path/to/JUCE`

**Missing dependencies (Linux):**
- Install all required development packages (see Linux setup)

**Build fails with C++17 errors:**
- Ensure your compiler supports C++17
- Update CMake to 3.15 or later

**Plugin not recognized by DAW:**
- Check plugin is in correct directory
- Verify plugin format compatibility
- Restart DAW and rescan plugins

### Debug Builds
For debugging and development:
```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build .
```

### Verbose Build Output
```bash
cmake --build . --verbose
```

## Testing

### Unit Tests
```bash
# Build and run tests
cmake .. -DBUILD_TESTS=ON
cmake --build .
ctest
```

### Plugin Validation
Test the plugin in various DAWs:
- Logic Pro (macOS)
- Pro Tools
- Ableton Live
- Reaper
- Cubase

## Performance Optimization

### Release Builds
Always use Release configuration for final builds:
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release
```

### CPU Optimization
Enable platform-specific optimizations:
```bash
cmake .. -DENABLE_NATIVE_OPTIMIZATION=ON
```

## Packaging

### Create Installer
```bash
cmake --build . --target package
```

This creates platform-specific installers in the build directory.

## Development

### Code Formatting
Use clang-format with the provided configuration:
```bash
clang-format -i Source/**/*.cpp Source/**/*.h
```

### Static Analysis
```bash
# Using clang-tidy
clang-tidy Source/**/*.cpp
```

## Support

For build issues or questions:
- Check the [GitHub Issues](https://github.com/yourcompany/reverb-plugin/issues)
- Consult the [JUCE Documentation](https://docs.juce.com/)
- Visit the [JUCE Forum](https://forum.juce.com/)

## License

This project is licensed under the GPL v3 License - see the LICENSE file for details.