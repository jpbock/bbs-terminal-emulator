# BBS Terminal Emulator - API Reference

Complete API documentation for all public interfaces.

## Core Modules

### Telnet Client (telnet_client.h)

```cpp
namespace telnet {

class TelnetClient {
public:
    // Connection management
    bool connect(const std::string& host, int port);
    void disconnect();
    bool is_connected() const;

    // Data transmission
    void send_data(const std::vector<unsigned char>& data);
    void send_text(const std::string& text);

    // Telnet negotiation
    void send_will(TelnetOption option);
    void send_wont(TelnetOption option);
    void send_do(TelnetOption option);
    void send_dont(TelnetOption option);
    void set_window_size(int width, int height);

    // Callbacks
    void set_data_callback(DataCallback cb);
    void set_connect_callback(ConnectCallback cb);
    void set_disconnect_callback(DisconnectCallback cb);
};

} // namespace telnet
```

**Example:**
```cpp
telnet::TelnetClient client;
client.set_data_callback([](const auto& data) {
    std::cout << "Received " << data.size() << " bytes\n";
});
client.connect("bbs.example.com", 23);
client.send_text("Hello BBS!\r\n");
```

### ANSI Interpreter (ansi_interpreter.h)

```cpp
namespace terminal {

class ANSIInterpreter {
public:
    ANSIInterpreter(int width = 80, int height = 25);

    // Input processing
    void process_input(const std::vector<unsigned char>& data);

    // Callbacks
    void set_output_callback(OutputCallback cb);
    void set_clear_callback(ClearCallback cb);
    void set_cursor_callback(CursorCallback cb);

    // State queries
    int get_cursor_x() const;
    int get_cursor_y() const;
    int get_width() const;
    int get_height() const;

    // Display control
    void set_window_size(int width, int height);
    void clear_screen();
};

} // namespace terminal
```

**Supported ANSI Codes:**
- Cursor positioning: `CSI H` (cursor move)
- Text attributes: `CSI m` (SGR - Select Graphic Rendition)
- Colors: 30-37 (foreground), 40-47 (background)
- Attributes: bold, dim, italic, underline, blink, reverse, strikethrough
- Erase: `CSI J` (display), `CSI K` (line)

### Screen Buffer (screen_buffer.h)

```cpp
namespace terminal {

class ScreenBuffer {
public:
    ScreenBuffer(int width = 80, int height = 25);

    // Character manipulation
    void put_char(unsigned char ch, int col, int row, const TextAttribute& attr);
    void put_string(const std::string& str, int col, int row, const TextAttribute& attr);

    // Display control
    void clear();
    void scroll_up(int lines = 1);
    void scroll_down(int lines = 1);
    void resize(int width, int height);

    // State queries
    CharacterCell get_cell(int col, int row) const;
    std::string get_line(int row) const;
    std::string get_all_text() const;
    const std::vector<std::vector<CharacterCell>>& get_buffer() const;

    int get_width() const;
    int get_height() const;
};

} // namespace terminal
```

### File Transfer Manager (file_transfer_manager.h)

```cpp
namespace filetransfer {

class FileTransferManager {
public:
    // Transfer control
    bool start_send(ProtocolType protocol, const std::vector<std::string>& files);
    bool start_receive(ProtocolType protocol, const std::string& save_directory);
    void cancel_transfer();
    void process_data(const std::vector<unsigned char>& data);

    // Callbacks
    void set_send_data_callback(DataCallback cb);
    void set_progress_callback(ProgressCallback cb);
    void set_state_callback(StateCallback cb);
    void set_error_callback(ErrorCallback cb);

    // State queries
    TransferState get_state() const;
    ProtocolType get_protocol() const;
    const FileInfo& get_current_file() const;
};

} // namespace filetransfer
```

**Protocol Types:**
- `ProtocolType::XMODEM` - 128-byte blocks
- `ProtocolType::XMODEM_1K` - 1024-byte blocks
- `ProtocolType::YMODEM` - Multi-file, 1024-byte blocks
- `ProtocolType::YMODEM_G` - Streaming variant
- `ProtocolType::ZMODEM` - Full-duplex, 8192-byte blocks

### IEMSI Handler (iemsi_handler.h)

```cpp
namespace iemsi {

class IEMSIHandler {
public:
    // Control
    void initiate_iemsi_request();
    void send_iemsi_request(const IEMSIRequest& request);
    void process_data(const std::vector<unsigned char>& data);

    // Profile management
    void set_user_profile(const IEMSIUserProfile& profile);
    const IEMSIUserProfile& get_user_profile() const;

    // Callbacks
    void set_send_data_callback(DataCallback cb);
    void set_login_callback(LoginCallback cb);
    void set_error_callback(ErrorCallback cb);
    void set_state_callback(StateCallback cb);

    // State queries
    bool is_iemsi_active() const;
    bool is_user_logged_in() const;
    const IEMSIResponse& get_bbs_response() const;

    // Feature control
    void enable_iemsi();
    void disable_iemsi();
    bool is_iemsi_enabled() const;
};

} // namespace iemsi
```

### Phonebook (phonebook.h)

```cpp
namespace dialing {

class Phonebook {
public:
    // Entry management
    void add_entry(const DialEntry& entry);
    void update_entry(size_t index, const DialEntry& entry);
    void remove_entry(size_t index);
    void clear();

    // Queries
    const DialEntry& get_entry(size_t index) const;
    size_t get_entry_count() const;
    const std::vector<DialEntry>& get_all_entries() const;

    // Search
    std::vector<size_t> search_by_name(const std::string& pattern) const;
    std::vector<size_t> search_by_host(const std::string& pattern) const;

    // Persistence
    bool load_from_file(const std::string& filename);
    bool save_to_file(const std::string& filename) const;

    // Statistics
    void log_connection(size_t index);
    int get_times_called(size_t index) const;
};

} // namespace dialing
```

### Settings (settings.h)

```cpp
namespace config {

class Settings {
public:
    // Window settings
    void set_window_width(int width);
    void set_window_height(int height);
    void set_font_name(const std::string& font);
    void set_font_size(int size);
    int get_window_width() const;
    int get_window_height() const;
    const std::string& get_font_name() const;
    int get_font_size() const;

    // Terminal settings
    void set_terminal_width(int width);
    void set_terminal_height(int height);
    void set_terminal_type(const std::string& type);
    int get_terminal_width() const;
    int get_terminal_height() const;
    const std::string& get_terminal_type() const;

    // Colors
    void set_foreground_color(unsigned int color);
    void set_background_color(unsigned int color);
    unsigned int get_foreground_color() const;
    unsigned int get_background_color() const;

    // Features
    void set_enable_sound(bool enable);
    void set_enable_rip(bool enable);
    void set_enable_logging(bool enable);
    bool is_sound_enabled() const;
    bool is_rip_enabled() const;
    bool is_logging_enabled() const;

    // Persistence
    bool load_from_file(const std::string& filename);
    bool save_to_file(const std::string& filename) const;
};

} // namespace config
```

## Data Structures

### TextAttribute

```cpp
struct TextAttribute {
    Color foreground;          // RGB color
    Color background;          // RGB color
    bool bold;                 // Bold text
    bool dim;                  // Dimmed text
    bool italic;               // Italic text
    bool underline;            // Underlined text
    bool blink;                // Blinking text
    bool reverse;              // Reverse video
    bool hidden;               // Hidden text
    bool strikethrough;        // Struck-through text
};
```

### CharacterCell

```cpp
struct CharacterCell {
    unsigned char ch;          // Character code
    TextAttribute attr;        // Text attributes
};
```

### DialEntry

```cpp
struct DialEntry {
    std::string name;          // BBS name
    std::string host;          // Host address
    int port;                  // Port number (default 23)
    std::string username;      // Username for auto-login
    std::string password;      // Password for auto-login
    std::string terminal_type; // Terminal type (default "ANSI")
    bool auto_login;           // Auto-login enabled
    std::string comments;      // User notes
    std::chrono::system_clock::time_point last_call;
    int times_called;          // Call statistics
};
```

### FileInfo

```cpp
struct FileInfo {
    std::string filename;      // File name only
    uint32_t filesize;         // File size in bytes
    uint32_t modification_time;// Unix timestamp
    std::string full_path;     // Full file path
    uint32_t bytes_transferred;// Progress
};
```

### IEMSIUserProfile

```cpp
struct IEMSIUserProfile {
    std::string user_name;     // Login username
    std::string real_name;     // Real name
    std::string alias;         // Alias
    std::string email_address; // Email
    std::string phone_number;  // Phone number
    std::string terminal_type; // Terminal type (ANSI, VT100, RIP)
    uint16_t terminal_width;   // Terminal columns
    uint16_t terminal_height;  // Terminal rows
    bool supports_color;       // Color support
    bool supports_graphics;    // Graphics support
    IEMSICapabilities capabilities;
};
```

## Callback Types

### Telnet Callbacks

```cpp
using DataCallback = std::function<void(const std::vector<unsigned char>&)>;
using ConnectCallback = std::function<void(bool)>;  // success flag
using DisconnectCallback = std::function<void()>;
```

### Terminal Callbacks

```cpp
using OutputCallback = std::function<void(const CharacterCell&, int col, int row)>;
using ClearCallback = std::function<void()>;
using CursorCallback = std::function<void(int col, int row)>;
```

### File Transfer Callbacks

```cpp
using ProgressCallback = std::function<void(const FileInfo&, int percent)>;
using StateCallback = std::function<void(TransferState)>;
using ErrorCallback = std::function<void(const std::string&)>;
```

### IEMSI Callbacks

```cpp
using LoginCallback = std::function<void(const IEMSIUserProfile&)>;
using StateCallback = std::function<void(const std::string&)>;
```

## Error Handling

All components use callbacks for error reporting:

```cpp
setup_error_callback([](const std::string& error) {
    // Log error
    std::cerr << "Error: " << error << std::endl;
});
```

## Thread Safety

- **Telnet client:** Thread-safe for concurrent send/receive
- **Screen buffer:** Not thread-safe (use mutex for multi-threaded access)
- **Settings/Phonebook:** Thread-safe for reads, serialize writes
- **File transfer:** Thread-safe for state queries

## Memory Requirements

- Screen buffer (80×25): ~2KB
- File transfer buffer (64KB): 64KB
- IEMSI handler: ~1KB
- Telnet client: ~4KB
- **Total minimum:** ~70KB

## Versioning

- **API Version:** 1.0
- **ABI Compatibility:** Not guaranteed between versions
- **Deprecation:** 2 major versions notice before removal

## Examples

See `examples/` directory for complete code samples:
- `xmodem_transfer.cpp` - File transfer example
- `iemsi_login.cpp` - IEMSI login flow
- `ansi_rendering.cpp` - Terminal rendering
