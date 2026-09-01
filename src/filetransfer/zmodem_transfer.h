#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <functional>
#include <map>

namespace filetransfer {

class ZmodemTransfer {
public:
    using DataCallback = std::function<void(const std::vector<unsigned char>&)>;
    using ProgressCallback = std::function<void(uint32_t, uint32_t)>;
    using CompleteCallback = std::function<void(bool)>;
    using FileListCallback = std::function<void(const std::vector<std::string>&)>;

    ZmodemTransfer();
    ~ZmodemTransfer();

    // Send operations
    bool start_send(const std::vector<std::string>& files);
    void process_receive_data(const std::vector<unsigned char>& data);

    // Receive operations
    bool start_receive(const std::string& save_directory);
    void send_init();

    // Callbacks
    void set_send_callback(DataCallback cb) { send_callback_ = cb; }
    void set_progress_callback(ProgressCallback cb) { progress_callback_ = cb; }
    void set_complete_callback(CompleteCallback cb) { complete_callback_ = cb; }
    void set_file_list_callback(FileListCallback cb) { file_list_callback_ = cb; }

    void cancel();
    bool is_active() const { return active_; }

private:
    // Zmodem protocol constants
    static constexpr unsigned char ZDLE = 0x18;      // Data link escape
    static constexpr unsigned char ZBIN = 0x41;      // Binary frame indicator
    static constexpr unsigned char ZHEX = 0x42;      // Hex frame indicator
    static constexpr unsigned char ZPAD = 0x2A;      // Padding character '*'

    // Frame types
    enum class FrameType : unsigned char {
        ZRQINIT = 0,   // Request initialization
        ZRINIT = 1,    // Receiver initialization
        ZSINIT = 2,    // Sender initialization
        ZACK = 3,      // Acknowledge
        ZFILE = 4,     // File information
        ZSKIP = 5,     // Skip file
        ZNAK = 6,      // Negative acknowledge
        ZABORT = 7,    // Abort transfer
        ZFIN = 8,      // Finish
        ZRPOS = 9,     // Resume position
        ZDATA = 10,    // Data frame
        ZEOF = 11,     // End of file
        ZFERR = 12,    // File error
        ZCRC = 13,     // CRC request
        ZCHALLENGE = 14, // Challenge
        ZCOMPL = 15,   // Complete
        ZCAN = 16,     // Cancel
        ZFREECNT = 17, // Free count
        ZCOMMAND = 18  // Remote command
    };

    bool active_ = false;
    bool sending_ = false;
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

    void send_frame(FrameType type, const std::vector<unsigned char>& data);
    void send_file_header();
    void send_file_data();
    void send_finish();

    std::vector<unsigned char> encode_zdle(const std::vector<unsigned char>& data);
    std::vector<unsigned char> create_frame(FrameType type, const std::vector<unsigned char>& data);

    uint32_t calculate_crc32(const std::vector<unsigned char>& data, uint32_t crc = 0);
    uint16_t calculate_crc16(const std::vector<unsigned char>& data);
};

} // namespace filetransfer
