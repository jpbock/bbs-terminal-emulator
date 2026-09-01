#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <functional>
#include "file_transfer_manager.h"

namespace filetransfer {

class XmodemTransfer {
public:
    using DataCallback = std::function<void(const std::vector<unsigned char>&)>;
    using ProgressCallback = std::function<void(uint32_t, uint32_t)>;
    using CompleteCallback = std::function<void(bool)>;

    XmodemTransfer(bool use_1k = false);
    ~XmodemTransfer();

    // Send operations
    bool start_send(const std::string& filename);
    void process_receive_data(const std::vector<unsigned char>& data);

    // Receive operations
    bool start_receive(const std::string& save_path);
    void send_start_signal();

    // Callbacks
    void set_send_callback(DataCallback cb) { send_callback_ = cb; }
    void set_progress_callback(ProgressCallback cb) { progress_callback_ = cb; }
    void set_complete_callback(CompleteCallback cb) { complete_callback_ = cb; }

    void cancel();
    bool is_active() const { return active_; }

private:
    // Protocol constants
    static constexpr unsigned char SOH = 0x01;   // Start of Heading (128-byte block)
    static constexpr unsigned char STX = 0x02;   // Start of Text (1024-byte block)
    static constexpr unsigned char EOT = 0x04;   // End of Transmission
    static constexpr unsigned char ACK = 0x06;   // Acknowledge
    static constexpr unsigned char NAK = 0x15;   // Negative Acknowledge
    static constexpr unsigned char CAN = 0x18;   // Cancel
    static constexpr unsigned char SUB = 0x1A;   // Substitute (EOF padding)

    bool use_1k_blocks_;
    bool active_ = false;
    bool sending_ = false;
    uint32_t block_number_ = 0;
    uint32_t file_offset_ = 0;
    uint32_t file_size_ = 0;

    std::string filename_;
    std::string save_path_;
    std::vector<unsigned char> file_buffer_;
    std::vector<unsigned char> current_block_;

    DataCallback send_callback_;
    ProgressCallback progress_callback_;
    CompleteCallback complete_callback_;

    void send_block();
    void send_eot();
    std::vector<unsigned char> create_block(uint32_t block_num, const std::vector<unsigned char>& data);
    uint8_t calculate_checksum(const std::vector<unsigned char>& data);
    bool verify_block(const std::vector<unsigned char>& data);
};

} // namespace filetransfer
