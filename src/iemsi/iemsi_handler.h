#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>

namespace iemsi {

// IEMSI Protocol Constants
static constexpr const char* IEMSI_REQ = "**IEMSI_REQ**";
static constexpr const char* IEMSI_ACK = "**IEMSI_ACK**";
static constexpr const char* IEMSI_NAK = "**IEMSI_NAK**";
static constexpr const char* IEMSI_EVNT = "**IEMSI_EVNT";
static constexpr const char* IEMSI_CHT = "**IEMSI_CHT";
static constexpr unsigned char IEMSI_COMPL_CODE = 0xFF;

// IEMSI Capabilities flags
struct IEMSICapabilities {
    bool supports_terminal = false;      // Terminal emulation
    bool supports_file_transfer = false; // File transfer
    bool supports_chat = false;          // Chat capability
    bool supports_file_request = false;  // File request
    bool supports_voice = false;         // Voice capability
    bool supports_newsreader = false;    // Newsreader
    bool supports_archiver = false;      // Archive handling
    bool supports_modem = false;         // Modem capability
};

// User profile information
struct IEMSIUserProfile {
    std::string user_name;
    std::string real_name;
    std::string alias;
    std::string password;           // Usually not transmitted
    std::string email_address;
    std::string phone_number;
    std::string last_call;
    std::string bbs_name;
    std::string bbs_city;
    uint32_t baud_rate = 0;
    uint16_t call_count = 0;
    bool new_user = false;
    bool deleted_user = false;

    // Terminal capabilities
    std::string terminal_type;      // ANSI, VT100, RIP, etc.
    uint16_t terminal_width = 80;
    uint16_t terminal_height = 24;
    bool supports_color = true;
    bool supports_graphics = false;

    // System capabilities
    IEMSICapabilities capabilities;
};

struct IEMSIRequest {
    uint16_t request_id = 0;
    std::string requestor;          // Who is requesting (client identifier)
    uint32_t capabilities = 0;      // Capability flags
    uint32_t protocols = 0;         // Supported protocols (Xmodem, Ymodem, Zmodem, etc.)
    uint8_t max_data_block = 64;    // Max data block size (64-4096 bytes)
    uint8_t requests_iemsi = 1;     // Requests IEMSI response
};

struct IEMSIResponse {
    uint16_t response_id = 0;
    std::string bbs_name;
    std::string bbs_city;
    std::string system_operator;
    std::string node_number;
    uint32_t capabilities = 0;      // BBS capabilities
    uint32_t protocols = 0;         // Supported protocols
    uint8_t max_data_block = 64;
};

class IEMSIHandler {
public:
    IEMSIHandler();
    ~IEMSIHandler();

    using DataCallback = std::function<void(const std::vector<unsigned char>&)>;
    using LoginCallback = std::function<void(const IEMSIUserProfile&)>;
    using ErrorCallback = std::function<void(const std::string&)>;
    using StateCallback = std::function<void(const std::string&)>;

    // Initiate IEMSI handshake
    void initiate_iemsi_request();
    void send_iemsi_request(const IEMSIRequest& request);

    // Process incoming IEMSI data
    void process_data(const std::vector<unsigned char>& data);
    void process_command(const std::string& command);

    // Build user profile for login
    void set_user_profile(const IEMSIUserProfile& profile);
    const IEMSIUserProfile& get_user_profile() const { return user_profile_; }

    // Callbacks
    void set_send_data_callback(DataCallback cb) { send_data_callback_ = cb; }
    void set_login_callback(LoginCallback cb) { login_callback_ = cb; }
    void set_error_callback(ErrorCallback cb) { error_callback_ = cb; }
    void set_state_callback(StateCallback cb) { state_callback_ = cb; }

    // State queries
    bool is_iemsi_active() const { return iemsi_active_; }
    bool is_user_logged_in() const { return user_logged_in_; }
    const IEMSIResponse& get_bbs_response() const { return bbs_response_; }

    // Enable/disable IEMSI
    void enable_iemsi() { iemsi_enabled_ = true; }
    void disable_iemsi() { iemsi_enabled_ = false; }
    bool is_iemsi_enabled() const { return iemsi_enabled_; }

private:
    enum class IEMSIState {
        IDLE,
        REQUESTING,
        HANDSHAKING,
        WAITING_RESPONSE,
        AUTHENTICATED,
        FAILED
    };

    bool iemsi_enabled_ = true;
    bool iemsi_active_ = false;
    bool user_logged_in_ = false;
    IEMSIState state_ = IEMSIState::IDLE;

    IEMSIRequest current_request_;
    IEMSIResponse bbs_response_;
    IEMSIUserProfile user_profile_;

    std::string command_buffer_;
    std::vector<unsigned char> data_buffer_;

    DataCallback send_data_callback_;
    LoginCallback login_callback_;
    ErrorCallback error_callback_;
    StateCallback state_callback_;

    void set_state(IEMSIState state, const std::string& description = "");
    void process_iemsi_ack();
    void process_iemsi_nak();
    void process_iemsi_evnt(const std::string& event_data);
    void build_iemsi_request_packet(const IEMSIRequest& request);
    std::string encode_iemsi_data(const std::string& data);
    std::string decode_iemsi_data(const std::string& data);
    uint16_t calculate_iemsi_crc(const std::string& data);
};

} // namespace iemsi
