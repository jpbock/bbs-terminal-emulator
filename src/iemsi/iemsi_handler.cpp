#include "iemsi_handler.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cstring>

namespace iemsi {

IEMSIHandler::IEMSIHandler() {
}

IEMSIHandler::~IEMSIHandler() {
}

void IEMSIHandler::initiate_iemsi_request() {
    if (!iemsi_enabled_) {
        return;
    }

    // Build default request
    IEMSIRequest request;
    request.requestor = "BBSTerminal/1.0";
    request.capabilities = 0x0001;  // Supports terminal emulation
    request.protocols = 0x000F;     // Supports Xmodem, Ymodem, Zmodem, etc.
    request.max_data_block = 64;
    request.requests_iemsi = 1;

    send_iemsi_request(request);
}

void IEMSIHandler::send_iemsi_request(const IEMSIRequest& request) {
    current_request_ = request;
    set_state(IEMSIState::REQUESTING, "Sending IEMSI request");

    std::string packet = "**IEMSI_REQ**";

    // Add request data in format: ID,Requestor,Capabilities,Protocols,MaxBlock
    std::ostringstream oss;
    oss << std::hex << std::setfill('0')
        << std::setw(4) << request.request_id << ","
        << request.requestor << ","
        << std::setw(8) << request.capabilities << ","
        << std::setw(8) << request.protocols << ","
        << std::setw(2) << (int)request.max_data_block;

    packet += oss.str();

    // Calculate and append CRC
    uint16_t crc = calculate_iemsi_crc(oss.str());
    packet += std::string(1, (char)(crc & 0xFF));
    packet += std::string(1, (char)((crc >> 8) & 0xFF));

    // Send packet
    std::vector<unsigned char> data(packet.begin(), packet.end());
    if (send_data_callback_) {
        send_data_callback_(data);
    }
}

void IEMSIHandler::process_data(const std::vector<unsigned char>& data) {
    // Add data to buffer
    for (unsigned char byte : data) {
        if (byte == 0x0A || byte == 0x0D) {  // Line terminator
            if (!command_buffer_.empty()) {
                process_command(command_buffer_);
                command_buffer_.clear();
            }
        } else if (byte >= 32 && byte < 127) {  // Printable character
            command_buffer_ += (char)byte;
        }
    }
}

void IEMSIHandler::process_command(const std::string& command) {
    if (command.find("**IEMSI_ACK**") == 0) {
        process_iemsi_ack();
    } else if (command.find("**IEMSI_NAK**") == 0) {
        process_iemsi_nak();
    } else if (command.find("**IEMSI_EVNT") == 0) {
        std::string event_data = command.substr(12);
        process_iemsi_evnt(event_data);
    } else if (command.find("**IEMSI_CHT") == 0) {
        set_state(IEMSIState::AUTHENTICATED, "Chat request received");
    }
}

void IEMSIHandler::process_iemsi_ack() {
    iemsi_active_ = true;
    set_state(IEMSIState::HANDSHAKING, "BBS acknowledged IEMSI");

    // BBS supports IEMSI, now send login information
    if (!user_profile_.user_name.empty()) {
        std::string login_packet = "**IEMSI_REQ**";
        std::ostringstream oss;
        oss << user_profile_.user_name << ","
            << user_profile_.password << ","
            << user_profile_.email_address << ","
            << user_profile_.terminal_type << ","
            << std::hex << user_profile_.terminal_width << "x"
            << user_profile_.terminal_height;

        login_packet += oss.str();

        uint16_t crc = calculate_iemsi_crc(oss.str());
        login_packet += std::string(1, (char)(crc & 0xFF));
        login_packet += std::string(1, (char)((crc >> 8) & 0xFF));

        std::vector<unsigned char> data(login_packet.begin(), login_packet.end());
        if (send_data_callback_) {
            send_data_callback_(data);
        }
    }
}

void IEMSIHandler::process_iemsi_nak() {
    set_state(IEMSIState::FAILED, "BBS does not support IEMSI");
    if (error_callback_) {
        error_callback_("IEMSI negotiation failed - BBS sent NAK");
    }
}

void IEMSIHandler::process_iemsi_evnt(const std::string& event_data) {
    // Parse event data
    // Format: USER,<username>|PASSWORD|EMAIL,<email>|CAPABILITY,<flags>|...

    std::istringstream iss(event_data);
    std::string token;

    while (std::getline(iss, token, '|')) {
        size_t comma_pos = token.find(',');
        if (comma_pos != std::string::npos) {
            std::string key = token.substr(0, comma_pos);
            std::string value = token.substr(comma_pos + 1);

            if (key == "USER") {
                user_profile_.user_name = value;
                set_state(IEMSIState::AUTHENTICATED, "User authenticated: " + value);
                user_logged_in_ = true;
                if (login_callback_) {
                    login_callback_(user_profile_);
                }
            } else if (key == "ALIAS") {
                user_profile_.alias = value;
            } else if (key == "EMAIL") {
                user_profile_.email_address = value;
            } else if (key == "REAL_NAME") {
                user_profile_.real_name = value;
            } else if (key == "PHONE") {
                user_profile_.phone_number = value;
            } else if (key == "TERMINAL") {
                user_profile_.terminal_type = value;
            } else if (key == "CAPABILITY") {
                // Parse capability flags
                try {
                    uint32_t caps = std::stoul(value, nullptr, 16);
                    user_profile_.capabilities.supports_terminal = (caps & 0x0001) != 0;
                    user_profile_.capabilities.supports_file_transfer = (caps & 0x0002) != 0;
                    user_profile_.capabilities.supports_chat = (caps & 0x0004) != 0;
                    user_profile_.capabilities.supports_file_request = (caps & 0x0008) != 0;
                } catch (...) {
                    // Ignore parsing errors
                }
            }
        }
    }
}

void IEMSIHandler::set_user_profile(const IEMSIUserProfile& profile) {
    user_profile_ = profile;
}

void IEMSIHandler::set_state(IEMSIState state, const std::string& description) {
    state_ = state;
    if (state_callback_) {
        std::string state_str;
        switch (state) {
            case IEMSIState::IDLE:
                state_str = "IDLE";
                break;
            case IEMSIState::REQUESTING:
                state_str = "REQUESTING";
                break;
            case IEMSIState::HANDSHAKING:
                state_str = "HANDSHAKING";
                break;
            case IEMSIState::WAITING_RESPONSE:
                state_str = "WAITING_RESPONSE";
                break;
            case IEMSIState::AUTHENTICATED:
                state_str = "AUTHENTICATED";
                break;
            case IEMSIState::FAILED:
                state_str = "FAILED";
                break;
        }

        std::string full_msg = state_str;
        if (!description.empty()) {
            full_msg += ": " + description;
        }
        state_callback_(full_msg);
    }
}

uint16_t IEMSIHandler::calculate_iemsi_crc(const std::string& data) {
    uint16_t crc = 0xFFFF;

    for (unsigned char byte : data) {
        crc ^= (uint16_t)byte;
        for (int i = 0; i < 8; i++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc = crc >> 1;
            }
        }
    }

    return crc;
}

std::string IEMSIHandler::encode_iemsi_data(const std::string& data) {
    // Simple encoding for IEMSI data
    std::string encoded;
    for (unsigned char c : data) {
        if (c < 32 || c > 126) {
            encoded += '~';
            encoded += (char)((c >> 4) + '@');
            encoded += (char)((c & 0x0F) + '@');
        } else if (c == '~') {
            encoded += "~~";
        } else {
            encoded += c;
        }
    }
    return encoded;
}

std::string IEMSIHandler::decode_iemsi_data(const std::string& data) {
    std::string decoded;
    for (size_t i = 0; i < data.length(); ++i) {
        if (data[i] == '~') {
            if (i + 1 < data.length() && data[i + 1] == '~') {
                decoded += '~';
                i++;
            } else if (i + 2 < data.length()) {
                unsigned char hi = data[i + 1] - '@';
                unsigned char lo = data[i + 2] - '@';
                decoded += (char)((hi << 4) | lo);
                i += 2;
            }
        } else {
            decoded += data[i];
        }
    }
    return decoded;
}

} // namespace iemsi
