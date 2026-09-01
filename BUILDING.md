# Building BBS Terminal Emulator

## Prerequisites

### Required Software

1. **Visual Studio 2022** (Community Edition or later)
   - C++ Desktop Development workload
   - CMake tools for Visual Studio
   - Visual C++ build tools

2. **CMake** 3.20 or later
   - Download from https://cmake.org/download/
   - Add to PATH

3. **Git** (for cloning and version control)
   - Download from https://git-scm.com/

### Optional Dependencies

- **vcpkg** - For dependency management (recommended)
  ```bash
  git clone https://github.com/Microsoft/vcpkg.git
  cd vcpkg
  .\bootstrap-vcpkg.bat
  ```

## Building from Source

### Method 1: Using Visual Studio 2022 (Recommended)

1. **Clone the repository:**
   ```bash
   git clone https://github.com/jpbock/bbs-terminal-emulator.git
   cd bbs-terminal-emulator
   ```

2. **Open in Visual Studio:**
   - Launch Visual Studio 2022
   - Select "Open a project or solution"
   - Navigate to the cloned directory
   - Visual Studio will automatically detect CMakeLists.txt

3. **Configure CMake:**
   - Visual Studio will prompt to configure CMake
   - Select your preferred configuration (Debug/Release)

4. **Build:**
   - Go to Build → Build All (Ctrl+Shift+B)
   - Or select Build → Build bbs-terminal from the menu

5. **Run:**
   - Select Debug → Start Without Debugging (Ctrl+F5)
   - Or press F5 for debugging

### Method 2: Command Line Build

1. **Clone and setup:**
   ```bash
   git clone https://github.com/jpbock/bbs-terminal-emulator.git
   cd bbs-terminal-emulator
   ```

2. **Create build directory:**
   ```bash
   mkdir build
   cd build
   ```

3. **Configure with CMake:**
   ```bash
   # For Visual Studio 2022 (64-bit)
   cmake .. -G "Visual Studio 17 2022" -A x64
   
   # For Visual Studio 2022 (32-bit)
   cmake .. -G "Visual Studio 17 2022" -A Win32
   ```

4. **Build:**
   ```bash
   cmake --build . --config Release
   ```

5. **Run:**
   ```bash
   Release\bbs-terminal.exe
   ```

## Dependency Installation

### Using vcpkg (Recommended)

```bash
# Set vcpkg integration in Visual Studio
.\vcpkg\vcpkg integrate install

# Install jsoncpp for configuration management
.\vcpkg\vcpkg install jsoncpp:x64-windows
.\vcpkg\vcpkg install jsoncpp:x86-windows
```

Then update CMakeLists.txt to include vcpkg:
```cmake
set(CMAKE_TOOLCHAIN_FILE "path/to/vcpkg/scripts/buildsystems/vcpkg.cmake")
```

## Troubleshooting

### Issue: CMake not found

**Solution:** Ensure CMake is installed and added to PATH:
```bash
cmake --version
```

### Issue: Visual Studio generator not found

**Solution:** Verify Visual Studio 2022 installation:
```bash
where cl.exe
```

If not found, add to PATH or use developer command prompt.

### Issue: Missing Windows SDK

**Solution:** Install Windows SDK through Visual Studio Installer:
1. Open Visual Studio Installer
2. Select "Modify" on your Visual Studio installation
3. Check "Windows 10/11 SDK" under Optional components

### Issue: Linker errors with ws2_32

**Solution:** Ensure Windows SDK is properly installed. If using vcpkg, reinstall CMake integration:
```bash
.\vcpkg\vcpkg integrate install
```

## Build Output

After successful build:

- **Debug:** `build\Debug\bbs-terminal.exe`
- **Release:** `build\Release\bbs-terminal.exe`

## Development Tips

### Visual Studio Configuration

1. **Enable IntelliSense:** Project → Rescan Solution
2. **Configure debugger:** Debug → Windows → Registers
3. **Set breakpoints:** Click left margin or press F9

### Command Line Development

```bash
# Build specific configuration
cmake --build . --config Debug
cmake --build . --config Release

# Clean build
cmake --build . --target clean

# Rebuild
cmake --build . --clean-first
```

### Code Formatting

The project uses C++20 standard. Format code with ClangFormat:
```bash
clang-format -i src/**/*.cpp src/**/*.h
```

## Performance Builds

For optimized Release builds:

```bash
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release -- /p:Configuration=Release /p:Platform=x64
```

## Cross-Compilation

### 32-bit Build on 64-bit System

```bash
cmake .. -G "Visual Studio 17 2022" -A Win32
cmake --build . --config Release
```

## Next Steps

1. Review [README.md](README.md) for feature overview
2. Check [CONTRIBUTING.md](CONTRIBUTING.md) for development guidelines
3. Explore example configurations in `examples/` directory
4. Read the code architecture in key components

## Support

For build-related issues:
- Check Visual Studio version with `cl.exe /?`
- Verify CMake version with `cmake --version`
- Review CMake configuration output carefully
- Check [GitHub Issues](https://github.com/jpbock/bbs-terminal-emulator/issues)
