#include "telnet_client.h"
#include <winsock2.h>
#include <iostream>
#include <algorithm>

#pragma comment(lib, "ws2_32.lib")

namespace telnet {

TelnetClient::TelnetClient()
    : socket_handle_(INVALID_SOCKET), connected_(false) {
    // Initialize Winsock
    WSADATA wsa_data;
    WSAStartup(MAKEWORD(2, 2), &wsa_data);
}

TelnetClient::~TelnetClient() {
    disconnect();
    WSACleanup();
}

bool TelnetClient::connect(const std::string& host, int port) {
    if (connected_) return false;

    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        if (connect_callback_) connect_callback_(false);
        return false;
    }

    sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(host.c_str());

    if (server_addr.sin_addr.s_addr == INADDR_NONE) {
        hostent* host_ent = gethostbyname(host.c_str());
        if (!host_ent) {
            closesocket(sock);
            if (connect_callback_) connect_callback_(false);
            return false;
        }
        memcpy(&server_addr.sin_addr.s_addr, host_ent->h_addr, host_ent->h_length);
    }

    if (::connect(sock, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        closesocket(sock);
        if (connect_callback_) connect_callback_(false);
        return false;
    }

    socket_handle_ = (void*)sock;
    connected_ = true;

    // Start receive thread
    receive_thread_ = std::thread(&TelnetClient::receive_loop, this);

    if (connect_callback_) connect_callback_(true);
    return true;
}

void TelnetClient::disconnect() {
    if (!connected_) return;

    connected_ = false;
    SOCKET sock = (SOCKET)socket_handle_;
    if (sock != INVALID_SOCKET) {
        closesocket(sock);
        socket_handle_ = (void*)INVALID_SOCKET;
    }

    if (receive_thread_.joinable()) {
        receive_thread_.join();
    }

    if (disconnect_callback_) disconnect_callback_();
}

bool TelnetClient::is_connected() const {
    return connected_;
}

void TelnetClient::send_data(const std::vector<unsigned char>& data) {
    if (!connected_) return;

    std::lock_guard<std::mutex> lock(socket_mutex_);
    SOCKET sock = (SOCKET)socket_handle_;
    send(sock, (const char*)data.data(), (int)data.size(), 0);
}

void TelnetClient::send_text(const std::string& text) {
    std::vector<unsigned char> data(text.begin(), text.end());
    send_data(data);
}

void TelnetClient::send_will(TelnetOption option) {
    unsigned char cmd[] = {
        (unsigned char)TelnetCommand::IAC,
        (unsigned char)TelnetCommand::WILL,
        (unsigned char)option
    };
    send_data(std::vector<unsigned char>(cmd, cmd + 3));
}

void TelnetClient::send_wont(TelnetOption option) {
    unsigned char cmd[] = {
        (unsigned char)TelnetCommand::IAC,
        (unsigned char)TelnetCommand::WONT,
        (unsigned char)option
    };
    send_data(std::vector<unsigned char>(cmd, cmd + 3));
}

void TelnetClient::send_do(TelnetOption option) {
    unsigned char cmd[] = {
        (unsigned char)TelnetCommand::IAC,
        (unsigned char)TelnetCommand::DO,
        (unsigned char)option
    };
    send_data(std::vector<unsigned char>(cmd, cmd + 3));
}

void TelnetClient::send_dont(TelnetOption option) {
    unsigned char cmd[] = {
        (unsigned char)TelnetCommand::IAC,
        (unsigned char)TelnetCommand::DONT,
        (unsigned char)option
    };
    send_data(std::vector<unsigned char>(cmd, cmd + 3));
}

void TelnetClient::set_window_size(int width, int height) {
    unsigned char cmd[] = {
        (unsigned char)TelnetCommand::IAC,
        (unsigned char)TelnetCommand::SB,
        (unsigned char)TelnetOption::NAWS,
        (unsigned char)(width >> 8),
        (unsigned char)(width & 0xFF),
        (unsigned char)(height >> 8),
        (unsigned char)(height & 0xFF),
        (unsigned char)TelnetCommand::IAC,
        (unsigned char)TelnetCommand::SE
    };
    send_data(std::vector<unsigned char>(cmd, cmd + 9));
}

void TelnetClient::receive_loop() {
    const int BUFFER_SIZE = 4096;
    char buffer[BUFFER_SIZE];

    while (connected_) {
        SOCKET sock = (SOCKET)socket_handle_;
        int bytes_received = recv(sock, buffer, BUFFER_SIZE, 0);

        if (bytes_received <= 0) {
            disconnect();
            break;
        }

        std::vector<unsigned char> data(buffer, buffer + bytes_received);
        process_telnet_data(data);
    }
}

void TelnetClient::process_telnet_data(const std::vector<unsigned char>& data) {
    auto clean_data = parse_telnet_stream(data);
    if (!clean_data.empty() && data_callback_) {
        data_callback_(clean_data);
    }
}

std::vector<unsigned char> TelnetClient::parse_telnet_stream(const std::vector<unsigned char>& data) {
    std::vector<unsigned char> result;
    size_t i = 0;

    while (i < data.size()) {
        if (data[i] == (unsigned char)TelnetCommand::IAC && i + 1 < data.size()) {
            unsigned char cmd = data[i + 1];

            if (cmd == (unsigned char)TelnetCommand::DO) {
                if (i + 2 < data.size()) {
                    auto option = (TelnetOption)data[i + 2];
                    // Handle DO option
                    i += 3;
                } else break;
            } else if (cmd == (unsigned char)TelnetCommand::WILL) {
                if (i + 2 < data.size()) {
                    auto option = (TelnetOption)data[i + 2];
                    // Handle WILL option
                    i += 3;
                } else break;
            } else if (cmd == (unsigned char)TelnetCommand::SB) {
                // Skip subnegotiation
                i += 2;
                while (i < data.size() && !(data[i] == (unsigned char)TelnetCommand::IAC &&
                       i + 1 < data.size() && data[i + 1] == (unsigned char)TelnetCommand::SE)) {
                    i++;
                }
                if (i < data.size()) i += 2;
            } else {
                i += 2;
            }
        } else {
            result.push_back(data[i]);
            i++;
        }
    }

    return result;
}

} // namespace telnet
