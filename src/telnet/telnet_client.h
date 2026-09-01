#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <thread>
#include <mutex>
#include <queue>

namespace telnet {

enum class TelnetCommand : unsigned char {
    SE = 240,      // End of subnegotiation
    NOP = 241,     // No operation
    DM = 242,      // Data mark
    BRK = 243,     // Break
    IP = 244,      // Interrupt process
    AO = 245,      // Abort output
    AYT = 246,     // Are you there
    EC = 247,      // Erase character
    EL = 248,      // Erase line
    GA = 249,      // Go ahead
    SB = 250,      // Subnegotiation begin
    WILL = 251,    // Will
    WONT = 252,    // Won't
    DO = 253,      // Do
    DONT = 254,    // Don't
    IAC = 255      // Interpret as command
};

enum class TelnetOption : unsigned char {
    ECHO = 1,
    SGA = 3,       // Suppress Go Ahead
    STATUS = 5,
    TIMING_MARK = 6,
    NAWS = 31,     // Negotiate About Window Size
    TERMINAL_TYPE = 24,
    TERMINAL_SPEED = 32
};

class TelnetClient {
public:
    TelnetClient();
    ~TelnetClient();

    using DataCallback = std::function<void(const std::vector<unsigned char>&)>;
    using ConnectCallback = std::function<void(bool)>;
    using DisconnectCallback = std::function<void()>;

    bool connect(const std::string& host, int port);
    void disconnect();
    bool is_connected() const;

    void send_data(const std::vector<unsigned char>& data);
    void send_text(const std::string& text);

    // Telnet negotiation
    void send_will(TelnetOption option);
    void send_wont(TelnetOption option);
    void send_do(TelnetOption option);
    void send_dont(TelnetOption option);
    void set_window_size(int width, int height);

    // Callbacks
    void set_data_callback(DataCallback cb) { data_callback_ = cb; }
    void set_connect_callback(ConnectCallback cb) { connect_callback_ = cb; }
    void set_disconnect_callback(DisconnectCallback cb) { disconnect_callback_ = cb; }

private:
    void receive_loop();
    void process_telnet_data(const std::vector<unsigned char>& data);
    std::vector<unsigned char> parse_telnet_stream(const std::vector<unsigned char>& data);

    // Platform-specific socket
    void* socket_handle_;
    bool connected_;
    std::thread receive_thread_;
    std::mutex socket_mutex_;

    DataCallback data_callback_;
    ConnectCallback connect_callback_;
    DisconnectCallback disconnect_callback_;

    std::vector<unsigned char> telnet_buffer_;
};

} // namespace telnet
