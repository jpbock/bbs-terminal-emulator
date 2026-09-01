# BBS Terminal Emulator

A modern C++ BBS terminal emulator for Windows with telnet connectivity, ANSI art support, and RIP 1.54 graphics rendering. Similar in functionality to SyncTerm but with a modern Windows interface.

## Features

- **Telnet Protocol Support** - Full telnet negotiation with NAWS (Negotiate About Window Size) support
- **ANSI Art Rendering** - Complete ANSI escape sequence support including:
  - Cursor positioning and movement
  - Text attributes (bold, dim, underline, blink, reverse, strikethrough)
  - 16-color support with standard ANSI color codes
  - Text erasure commands

- **RIP 1.54 Graphics** - Remote Imaging Protocol support including:
  - Pixel-level drawing
  - Lines, rectangles, circles, polygons
  - Filled shapes
  - Text rendering with custom fonts
  - Palette management
  - Write modes

- **Dialing Directory** - Full-featured phonebook with:
  - Add/edit/delete BBS entries
  - Search by name or host
  - Auto-login credentials storage
  - Call statistics (times called, last call date)
  - JSON persistence

- **Configuration** - Comprehensive settings including:
  - Window dimensions and fonts
  - Terminal emulation parameters
  - Color schemes
  - Feature toggles (sound, RIP, logging)
  - JSON-based configuration file

- **Modern UI** - Clean Windows native interface with:
  - Tabbed sessions
  - Menu bar and status bar
  - Direct2D hardware-accelerated rendering
  - Clipboard integration

## System Requirements

- Windows 7 or later
- Visual Studio 2022 or later
- C++20 compatible compiler
- .NET Framework 4.8 or later (for some dependencies)

## Building

See [BUILDING.md](BUILDING.md) for detailed build instructions.

## Usage

### First Run

1. Launch the application
2. Open the Phonebook (File → Phonebook)
3. Add a BBS entry with the host and port
4. Click "Dial" to connect

### Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| Ctrl+C | Interrupt |
| Ctrl+X | Copy selection |
| Ctrl+V | Paste text |
| Ctrl+Q | Disconnect |
| Alt+F4 | Exit application |

### Configuration Files

**Phonebook:** `%APPDATA%\BBSTerminal\phonebook.json`

**Settings:** `%APPDATA%\BBSTerminal\settings.json`

**Logs:** `%APPDATA%\BBSTerminal\logs\`

## Architecture

The project is organized into functional namespaces:

- **telnet** - Low-level telnet protocol implementation
- **terminal** - ANSI and RIP interpreters, screen buffer management
- **ui** - Windows UI components and rendering
- **dialing** - Phonebook and connection management
- **config** - Settings and configuration management

## Project Structure

```
src/
├── main.cpp                 # Application entry point
├── telnet/
│   ├── telnet_client.h      # Telnet protocol client
│   └── telnet_client.cpp
├── terminal/
│   ├── ansi_interpreter.h   # ANSI escape sequence handling
│   ├── ansi_interpreter.cpp
│   ├── rip_interpreter.h    # RIP 1.54 graphics protocol
│   ├── rip_interpreter.cpp
│   ├── screen_buffer.h      # Character grid management
│   └── screen_buffer.cpp
├── ui/
│   ├── main_window.h        # Main application window
│   ├── main_window.cpp
│   ├── terminal_view.h      # Terminal rendering surface
│   └── terminal_view.cpp
├── dialing/
│   ├── phonebook.h          # Phonebook/dialing directory
│   └── phonebook.cpp
└── config/
    ├── settings.h           # Application settings
    └── settings.cpp
```

## Contributing

Contributions are welcome! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

## License

MIT License - see [LICENSE](LICENSE) file for details.

## Acknowledgments

Inspired by classic BBS terminal emulators like:
- SyncTerm
- Zterm
- Telix
- Procomm Plus

## Support

For issues, questions, or suggestions, please open an issue on GitHub.

## Roadmap

- [ ] Tabbed sessions
- [ ] Session logging and playback
- [ ] Zmodem/Ymodem file transfer
- [ ] Keyboard macro recording
- [ ] Script support (Lua/Python)
- [ ] Mouse support in terminal
- [ ] Theme customization UI
- [ ] Sixel graphics support
- [ ] Unicode support improvements
- [ ] Network proxy support
