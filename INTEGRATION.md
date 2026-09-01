# Integration Guide: BBS Terminal Emulator

This guide explains how to integrate the various components of BBS Terminal Emulator and build a complete BBS session.

## Architecture Overview

```
┌─────────────────────────────────────────────────────┐
│         Main Application (main_window.h)            │
├─────────────────────────────────────────────────────┤
│                                                     │
│  ┌──────────────────────────────────────────────┐  │
│  │      Telnet Client (telnet_client.h)         │  │
│  │  - Socket management                         │  │
│  │  - Protocol negotiation (NAWS, ECHO, etc.)   │  │
│  └──────────────────────────────────────────────┘  │
│                      ↓                              │
│  ┌──────────────────────────────────────────────┐  │
│  │   IEMSI Handler (iemsi_handler.h)            │  │
│  │  - BBS login automation                      │  │
│  │  - User profile management                   │  │
│  │  - Capability negotiation                    │  │
│  └──────────────────────────────────────────────┘  │
│                      ↓                              │
│  ┌──────────────────────────────────────────────┐  │
│  │   Terminal Interpreters                      │  │
│  │  ├─ ANSI Interpreter (ansi_interpreter.h)    │  │
│  │  ├─ RIP Interpreter (rip_interpreter.h)      │  │
│  │  └─ Screen Buffer (screen_buffer.h)          │  │
│  └──────────────────────────────────────────────┘  │
│                      ↓                              │
│  ┌──────────────────────────────────────────────┐  │
│  │   Terminal View (terminal_view.h)            │  │
│  │  - Direct2D rendering                        │  │
│  │  - Graphics display                          │  │
│  └──────────────────────────────────────────────┘  │
│                                                     │
│  ┌──────────────────────────────────────────────┐  │
│  │   File Transfer Manager                      │  │
│  │  ├─ Xmodem (xmodem_transfer.h)               │  │
│  │  ├─ Ymodem (ymodem_transfer.h)               │  │
│  │  └─ Zmodem (zmodem_transfer.h)               │  │
│  └──────────────────────────────────────────────┘  │
│                                                     │
│  ┌──────────────────────────────────────────────┐  │
│  │   Configuration & Phonebook                  │  │
│  │  ├─ Settings (settings.h)                    │  │
│  │  └─ Phonebook (phonebook.h)                  │  │
│  └──────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────┘
```

## Component Integration Flow

### 1. Application Startup

```cpp
// main.cpp
int main() {
    // Initialize COM
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    
    // Create and show main window
    ui::MainWindow main_window;
    main_window.create();
    main_window.show();
    
    // Run message loop
    return main_window.run();
}
```

### 2. Establishing Connection

```cpp
// main_window.cpp
void MainWindow::connect_to_bbs(const dialing::DialEntry& entry) {
    // 1. Create telnet connection
    telnet_client_->connect(entry.host, entry.port);
    
    // 2. Setup telnet callbacks
    telnet_client_->set_data_callback(
        [this](const std::vector<unsigned char>& data) {
            on_telnet_data_received(data);
        }
    );
    
    // 3. Initialize IEMSI if enabled
    iemsi_handler_.enable_iemsi();
    iemsi_handler_.initiate_iemsi_request();
}
```

### 3. Data Reception and Processing

```cpp
// main_window.cpp
void MainWindow::on_telnet_data_received(
    const std::vector<unsigned char>& data) {
    
    // 1. Try IEMSI handling first
    iemsi_handler_.process_data(data);
    
    // 2. Process terminal data
    ansi_interpreter_->process_input(data);
    rip_interpreter_->process_input(std::string(data.begin(), data.end()));
    
    // 3. Update screen buffer
    screen_buffer_->append_data(data);
    
    // 4. Render to display
    terminal_view_->render();
}
```

### 4. File Transfer Integration

```cpp
// Example: Starting a file transfer
void MainWindow::start_file_transfer(const std::string& filename) {
    // 1. Create file transfer manager
    filetransfer::FileTransferManager transfer_mgr;
    
    // 2. Setup callbacks
    transfer_mgr.set_send_data_callback(
        [this](const std::vector<unsigned char>& data) {
            telnet_client_->send_data(data);
        }
    );
    
    transfer_mgr.set_progress_callback(
        [this](const FileInfo& file, int percent) {
            update_status(file.filename + ": " + 
                         std::to_string(percent) + "%");
        }
    );
    
    // 3. Start transfer
    std::vector<std::string> files{filename};
    transfer_mgr.start_send(ProtocolType::ZMODEM, files);
}
```

## Configuration Management

### Settings File Structure

**Location:** `%APPDATA%\BBSTerminal\settings.json`

```json
{
  "window": {
    "width": 1024,
    "height": 768,
    "font": "Consolas",
    "font_size": 12
  },
  "terminal": {
    "width": 80,
    "height": 25,
    "type": "ANSI"
  },
  "colors": {
    "foreground": 16777215,
    "background": 0
  },
  "features": {
    "sound": true,
    "rip": true,
    "logging": false,
    "iemsi": true
  }
}
```

### Phonebook File Structure

**Location:** `%APPDATA%\BBSTerminal\phonebook.json`

```json
{
  "entries": [
    {
      "name": "Example BBS",
      "host": "example-bbs.net",
      "port": 23,
      "username": "guest",
      "password": "",
      "terminal_type": "ANSI",
      "auto_login": true,
      "comments": "Classic retro BBS",
      "times_called": 42
    }
  ]
}
```

## IEMSI Login Flow

```
Client                              BBS Server
  |                                  |
  |-------- **IEMSI_REQ** --------->|
  |                                  |
  |<-------- **IEMSI_ACK** ---------|
  |                                  |
  |-------- User Profile --------->|
  |                                  |
  |<----- **IEMSI_EVNT** (USER) ----|
  |                                  |
  |        [Connected & Logged In]   |
```

### IEMSI Handler Integration

```cpp
// Setup IEMSI
iemsi::IEMSIHandler iemsi_handler;
iemsi_handler.enable_iemsi();

// Set user profile
iemsi::IEMSIUserProfile profile;
profile.user_name = "YourUsername";
profile.real_name = "Your Real Name";
profile.email_address = "you@example.com";
profile.terminal_type = "ANSI";
profile.terminal_width = 80;
profile.terminal_height = 25;

iemsi_handler.set_user_profile(profile);

// Set callbacks
iemsi_handler.set_login_callback(
    [this](const iemsi::IEMSIUserProfile& profile) {
        update_status("Logged in as: " + profile.user_name);
    }
);

// Process telnet data through IEMSI
iemsi_handler.process_data(received_data);
```

## File Transfer Protocol Selection

### Xmodem (Simple, Basic)
- **Use for:** Simple single-file transfers
- **Block size:** 128 bytes
- **Checksum:** CRC-16

```cpp
filetransfer::XmodemTransfer xmodem;
xmodem.start_send("file.txt");
```

### Ymodem (Multi-file, Reliable)
- **Use for:** Multiple files, file information
- **Block size:** 1024 bytes
- **CRC:** CRC-16

```cpp
std::vector<std::string> files{"file1.txt", "file2.txt"};
filetransfer::YmodemTransfer ymodem;
ymodem.start_send(files);
```

### Zmodem (Full-duplex, Fast)
- **Use for:** Fast transfers, large files
- **Block size:** 8192 bytes
- **CRC:** CRC-32
- **Features:** Resume capability, full-duplex

```cpp
filetransfer::ZmodemTransfer zmodem;
zmodem.start_send(files);
```

## Terminal Rendering Pipeline

```
Raw Data (Telnet)
    ↓
ANSI Interpreter
    ├─ Parse escape sequences
    ├─ Extract colors & attributes
    └─ Update cursor position
    ↓
Screen Buffer
    ├─ Store characters with attributes
    ├─ Handle scrolling
    └─ Manage screen state
    ↓
Terminal View (Direct2D)
    ├─ Render character grid
    ├─ Apply colors
    ├─ Display RIP graphics
    └─ Update frame buffer
    ↓
Windows Display
```

## Callback Chain

### Telnet → Terminal → UI

```cpp
// 1. Telnet receives data
telnet_client_->set_data_callback([](const auto& data) {
    // 2. Pass to IEMSI
    iemsi_handler.process_data(data);
    
    // 3. Pass to terminal interpreters
    ansi_interpreter.process_input(data);
    rip_interpreter.process_input(data);
    
    // 4. Update screen buffer
    screen_buffer.append_data(data);
    
    // 5. Render to view
    terminal_view.render();
});
```

## Error Handling

### Connection Errors

```cpp
telnet_client_->set_error_callback([this](const std::string& error) {
    update_status("Error: " + error);
    disconnect();
});
```

### File Transfer Errors

```cpp
transfer_mgr.set_error_callback([this](const std::string& error) {
    MessageBox(hwnd_, error.c_str(), L"Transfer Error", MB_ICONERROR);
});
```

### IEMSI Errors

```cpp
iemsi_handler.set_error_callback([this](const std::string& error) {
    update_status("IEMSI Error: " + error);
    // Fall back to manual login
});
```

## Performance Optimization

### Threading Model

- **Main Thread:** UI updates, message loop
- **Telnet Thread:** Network I/O, data reception
- **Rendering Thread:** Terminal view updates (optional)

```cpp
// Telnet client runs in separate thread
telnet_client_->start_receive_thread();

// UI thread processes callbacks safely
SendMessage(hwnd_, WM_CUSTOM_DATA, 0, (LPARAM)data_ptr);
```

### Buffer Management

- Screen buffer sized to terminal dimensions (default 80×25)
- Direct2D render target uses hardware acceleration
- File transfer uses 64KB streaming buffers

## Memory Management

```cpp
// Use unique_ptr for automatic cleanup
std::unique_ptr<telnet::TelnetClient> telnet_client_;
std::unique_ptr<terminal::ANSIInterpreter> ansi_interpreter_;
std::unique_ptr<filetransfer::FileTransferManager> transfer_manager_;

// Destructors called automatically on scope exit
```

## Testing Integration

### Unit Test Example

```cpp
#include <cassert>

void test_iemsi_crc() {
    iemsi::IEMSIHandler handler;
    std::string test_data = "TEST";
    uint16_t crc = handler.calculate_iemsi_crc(test_data);
    assert(crc == 0x1234);  // Expected CRC value
}

void test_xmodem_block() {
    filetransfer::XmodemTransfer xmodem;
    std::vector<unsigned char> data(128, 'A');
    auto block = xmodem.create_block(1, data);
    assert(block.size() == 131);  // Header + data + checksum
}
```

## Troubleshooting

### IEMSI Not Detected

1. Check if BBS sends `**IEMSI_REQ**`
2. Verify IEMSI is enabled in settings
3. Check telnet negotiation completed
4. Review connection logs

### File Transfer Fails

1. Verify protocol support on BBS
2. Check CRC calculation
3. Monitor block retries
4. Verify file permissions

### Rendering Issues

1. Check terminal type setting
2. Verify ANSI interpretation
3. Confirm screen buffer size matches window
4. Check Direct2D device state

## Next Steps

See [API Reference](API_REFERENCE.md) for detailed component documentation.
