#include "ymodem_transfer.h"
#include <fstream>
#include <algorithm>
#include <windows.h>
#include <filesystem>

namespace fs = std::filesystem;

namespace filetransfer {

YmodemTransfer::YmodemTransfer(bool use_g_mode)
    : use_g_mode_(use_g_mode) {
}

YmodemTransfer::~YmodemTransfer() {
    cancel();
}

bool YmodemTransfer::start_send(const std::vector<std::string>& files) {
    files_ = files;
    sending_ = true;
    active_ = true;
    file_index_ = 0;
    block_number_ = 0;

    if (!files_.empty()) {
        // Load first file
        std::ifstream file(files_[0], std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            return false;
        }

        file_size_ = (uint32_t)file.tellg();
        file.seekg(0, std::ios::beg);
        file_buffer_.resize(file_size_);
        file.read((char*)file_buffer_.data(), file_size_);
        file.close();

        file_offset_ = 0;
    }

    return true;
}

bool YmodemTransfer::start_receive(const std::string& save_directory) {
    save_directory_ = save_directory;
    sending_ = false;
    active_ = true;
    block_number_ = 0;
    file_index_ = 0;

    send_start_signal();
    return true;
}

void YmodemTransfer::send_start_signal() {
    // Send CRC_INIT to request CRC mode
    std::vector<unsigned char> init{CRC_INIT};
    if (send_callback_) {
        send_callback_(init);
    }
}

void YmodemTransfer::process_receive_data(const std::vector<unsigned char>& data) {
    if (!active_) return;

    for (unsigned char byte : data) {
        if (byte == ACK) {
            if (sending_) {
                if (block_number_ == 0) {
                    // Header acknowledged, send file data
                    send_block();
                } else if (file_offset_ >= file_size_) {
                    // File completed
                    file_index_++;
                    if (file_index_ < files_.size()) {
                        // Load next file
                        std::ifstream file(files_[file_index_], std::ios::binary | std::ios::ate);
                        if (file.is_open()) {
                            file_size_ = (uint32_t)file.tellg();
                            file.seekg(0, std::ios::beg);
                            file_buffer_.resize(file_size_);
                            file.read((char*)file_buffer_.data(), file_size_);
                            file.close();

                            file_offset_ = 0;
                            block_number_ = 0;
                            send_file_header();
                        }
                    } else {
                        // All files sent, send end sequence
                        send_end_sequence();
                    }
                } else {
                    // Send next block
                    send_block();
                }
            }
        } else if (byte == NAK) {
            if (sending_) {
                if (block_number_ == 0) {
                    send_file_header();
                } else {
                    send_block();
                }
            }
        } else if (byte == CAN) {
            active_ = false;
            if (complete_callback_) complete_callback_(false);
        } else if (byte == CRC_INIT && !sending_ && block_number_ == 0) {
            // Receiver ready, send first file header
            send_file_header();
        }
    }
}

void YmodemTransfer::send_file_header() {
    if (file_index_ >= files_.size()) {
        return;
    }

    auto header = create_header_block(files_[file_index_], file_size_);
    if (send_callback_) {
        send_callback_(header);
    }
}

void YmodemTransfer::send_block() {
    if (file_offset_ >= file_size_) {
        return;
    }

    uint32_t block_size = 1024;
    uint32_t bytes_to_read = std::min(block_size, file_size_ - file_offset_);

    std::vector<unsigned char> block_data(
        file_buffer_.begin() + file_offset_,
        file_buffer_.begin() + file_offset_ + bytes_to_read
    );

    // Pad with EOF marker if necessary
    if (block_data.size() < block_size) {
        block_data.resize(block_size, EOF_MARKER);
    }

    block_number_++;
    auto block = create_data_block(block_number_, block_data);

    if (send_callback_) {
        send_callback_(block);
    }

    file_offset_ += bytes_to_read;

    if (progress_callback_) {
        uint32_t total_bytes = file_size_ * files_.size();
        uint32_t transferred = (file_index_ * file_size_) + file_offset_;
        uint32_t percent = (transferred * 100) / total_bytes;
        progress_callback_(transferred, percent);
    }
}

void YmodemTransfer::send_eot() {
    std::vector<unsigned char> eot{0x04};
    if (send_callback_) {
        send_callback_(eot);
    }
}

void YmodemTransfer::send_end_sequence() {
    // Send final EOT
    send_eot();
    active_ = false;
    if (complete_callback_) complete_callback_(true);
}

std::vector<unsigned char> YmodemTransfer::create_header_block(
    const std::string& filename,
    uint32_t filesize) {

    std::vector<unsigned char> block;
    block.push_back(SOH);
    block.push_back(0x00);  // Block 0 for header
    block.push_back(0xFF);

    // Add filename
    for (char c : filename) {
        if (c == '\0') break;
        block.push_back(c);
    }
    block.push_back(0x00);

    // Add filesize
    std::string size_str = std::to_string(filesize);
    for (char c : size_str) {
        block.push_back(c);
    }
    block.push_back(0x00);

    // Pad to 128 bytes
    while (block.size() < 131) {
        block.push_back(0x00);
    }

    // Calculate CRC
    std::vector<unsigned char> data(block.begin() + 3, block.begin() + 131);
    uint16_t crc = calculate_crc16(data);
    block.push_back((crc >> 8) & 0xFF);
    block.push_back(crc & 0xFF);

    return block;
}

std::vector<unsigned char> YmodemTransfer::create_data_block(
    uint32_t block_num,
    const std::vector<unsigned char>& data) {

    std::vector<unsigned char> block;
    block.push_back(SOH);
    block.push_back((block_num) & 0xFF);
    block.push_back(~(block_num) & 0xFF);

    block.insert(block.end(), data.begin(), data.end());

    uint16_t crc = calculate_crc16(data);
    block.push_back((crc >> 8) & 0xFF);
    block.push_back(crc & 0xFF);

    return block;
}

uint16_t YmodemTransfer::calculate_crc16(const std::vector<unsigned char>& data) {
    uint16_t crc = 0;
    for (unsigned char byte : data) {
        crc ^= (byte << 8);
        for (int i = 0; i < 8; i++) {
            crc <<= 1;
            if (crc & 0x10000) {
                crc ^= 0x1021;
            }
            crc &= 0xFFFF;
        }
    }
    return crc;
}

std::string YmodemTransfer::get_file_info(const std::string& filename) {
    try {
        auto path = fs::path(filename);
        auto size = fs::file_size(path);
        auto last_write = fs::last_write_time(path);
        return path.filename().string() + " " + std::to_string(size);
    } catch (...) {
        return "";
    }
}

void YmodemTransfer::cancel() {
    if (active_) {
        active_ = false;
        std::vector<unsigned char> cancel{CAN, CAN, CAN};
        if (send_callback_) {
            send_callback_(cancel);
        }
    }
}

} // namespace filetransfer
