#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <functional>
#include <map>

namespace filetransfer {

class YmodemTransfer {
public:
    using DataCallback = std::function<void(const std::vector<unsigned char>&)>;
    using ProgressCallback = std::function<void(uint32_t, uint32_t)>;
    using CompleteCallback = std::function<void(bool)>;
    using FileListCallback = std::function<void(const std::vector<std::string>&)>;

    YmodemTransfer(bool use_g_mode = false);
    ~YmodemTransfer();

    // Send operations
    bool start_send(const std::vector<std::string>& files);
    void process_receive_data(const std::vector<unsigned char>& data);

    // Receive operations
    bool start_receive(const std::string& save_directory);
    void send_start_signal();

    // Callbacks
    void set_send_callback(DataCallback cb) { send_callback_ = cb; }
    void set_progress_callback(ProgressCallback cb) { progress_callback_ = cb; }
    void set_complete_callback(CompleteCallback cb) { complete_callback_ = cb; }
    void set_file_list_callback(FileListCallback cb) { file_list_callback_ = cb; }

    void cancel();
    bool is_active() const { return active_; }

private:
    // Protocol constants
    static constexpr unsigned char SOH = 0x01;
    static constexpr unsigned char ACK = 0x06;
    static constexpr unsigned char NAK = 0x15;
    static constexpr unsigned char CAN = 0x18;
    static constexpr unsigned char EOF_MARKER = 0x1A;
    static constexpr unsigned char CRC_INIT = 'C';

    bool use_g_mode_;  // Ymodem-G mode (no error checking)
    bool active_ = false;
    bool sending_ = false;
    uint32_t block_number_ = 0;
    uint32_t file_index_ = 0;
    uint32_t file_offset_ = 0;
    uint32_t file_size_ = 0;

    std::vector<std::string> files_;
    std::string save_directory_;
    std::vector<unsigned char> file_buffer_;

    DataCallback send_callback_;
    ProgressCallback progress_callback_;
    CompleteCallback complete_callback_;
    FileListCallback file_list_callback_;

    void send_file_header();
    void send_block();
    void send_eot();
    void send_end_sequence();

    std::vector<unsigned char> create_header_block(const std::string& filename, uint32_t filesize);
    std::vector<unsigned char> create_data_block(uint32_t block_num, const std::vector<unsigned char>& data);

    uint16_t calculate_crc16(const std::vector<unsigned char>& data);
    std::string get_file_info(const std::string& filename);
};

} // namespace filetransfer
