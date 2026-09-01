#include "zmodem_transfer.h"
#include <fstream>
#include <algorithm>
#include <windows.h>
#include <filesystem>

namespace fs = std::filesystem;

namespace filetransfer {

ZmodemTransfer::ZmodemTransfer() {
}

ZmodemTransfer::~ZmodemTransfer() {
    cancel();
}

bool ZmodemTransfer::start_send(const std::vector<std::string>& files) {
    files_ = files;
    sending_ = true;
    active_ = true;
    file_index_ = 0;

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

    send_init();
    return true;
}

bool ZmodemTransfer::start_receive(const std::string& save_directory) {
    save_directory_ = save_directory;
    sending_ = false;
    active_ = true;
    file_index_ = 0;

    send_init();
    return true;
}

void ZmodemTransfer::send_init() {
    // Send ZRQINIT frame
    std::vector<unsigned char> data(4, 0);
    send_frame(FrameType::ZRQINIT, data);
}

void ZmodemTransfer::process_receive_data(const std::vector<unsigned char>& data) {
    if (!active_) return;

    // TODO: Implement Zmodem frame parsing and state machine
    // This is complex as Zmodem has full-duplex capability

    for (unsigned char byte : data) {
        if (byte == ZDLE) {
            // Frame escape - handle next byte
        }
    }
}

void ZmodemTransfer::send_frame(FrameType type, const std::vector<unsigned char>& data) {
    auto frame = create_frame(type, data);
    if (send_callback_) {
        send_callback_(frame);
    }
}

void ZmodemTransfer::send_file_header() {
    if (file_index_ >= files_.size()) {
        return;
    }

    std::string filename = files_[file_index_];
    std::vector<unsigned char> header_data;

    // Add filename
    for (char c : filename) {
        header_data.push_back(c);
    }
    header_data.push_back(0x00);

    // Add file size (4 bytes, little-endian)
    header_data.push_back((file_size_) & 0xFF);
    header_data.push_back((file_size_ >> 8) & 0xFF);
    header_data.push_back((file_size_ >> 16) & 0xFF);
    header_data.push_back((file_size_ >> 24) & 0xFF);

    send_frame(FrameType::ZFILE, header_data);
}

void ZmodemTransfer::send_file_data() {
    if (file_offset_ >= file_size_) {
        // Send ZEOF
        std::vector<unsigned char> eof_data(4);
        eof_data[0] = (file_size_) & 0xFF;
        eof_data[1] = (file_size_ >> 8) & 0xFF;
        eof_data[2] = (file_size_ >> 16) & 0xFF;
        eof_data[3] = (file_size_ >> 24) & 0xFF;
        send_frame(FrameType::ZEOF, eof_data);
        return;
    }

    // Send data block (max 8192 bytes per frame)
    uint32_t block_size = std::min(8192u, file_size_ - file_offset_);
    std::vector<unsigned char> block_data(
        file_buffer_.begin() + file_offset_,
        file_buffer_.begin() + file_offset_ + block_size
    );

    send_frame(FrameType::ZDATA, block_data);
    file_offset_ += block_size;

    if (progress_callback_) {
        uint32_t percent = (file_offset_ * 100) / file_size_;
        progress_callback_(file_offset_, percent);
    }
}

void ZmodemTransfer::send_finish() {
    std::vector<unsigned char> data(4, 0);
    send_frame(FrameType::ZFIN, data);
    active_ = false;
    if (complete_callback_) complete_callback_(true);
}

std::vector<unsigned char> ZmodemTransfer::encode_zdle(const std::vector<unsigned char>& data) {
    std::vector<unsigned char> encoded;
    for (unsigned char byte : data) {
        if (byte == ZDLE || byte == 0x0D || byte == 0x0A || byte == 0x00) {
            encoded.push_back(ZDLE);
            encoded.push_back(byte ^ 0x40);
        } else {
            encoded.push_back(byte);
        }
    }
    return encoded;
}

std::vector<unsigned char> ZmodemTransfer::create_frame(
    FrameType type,
    const std::vector<unsigned char>& data) {

    std::vector<unsigned char> frame;

    // Frame header
    frame.push_back(ZPAD);
    frame.push_back(ZDLE);
    frame.push_back(ZBIN);

    // Frame type
    frame.push_back((unsigned char)type);

    // Frame data
    auto encoded = encode_zdle(data);
    frame.insert(frame.end(), encoded.begin(), encoded.end());

    // CRC
    uint32_t crc = calculate_crc32(data);
    frame.push_back((crc) & 0xFF);
    frame.push_back((crc >> 8) & 0xFF);
    frame.push_back((crc >> 16) & 0xFF);
    frame.push_back((crc >> 24) & 0xFF);

    return frame;
}

uint32_t ZmodemTransfer::calculate_crc32(const std::vector<unsigned char>& data, uint32_t crc) {
    static const uint32_t crc32_table[256] = {
        // Standard CRC32 polynomial table
        0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,
        // ... (full CRC32 table omitted for brevity)
        0x00000000  // Placeholder
    };

    crc ^= 0xFFFFFFFF;
    for (unsigned char byte : data) {
        crc = (crc >> 8) ^ crc32_table[(crc ^ byte) & 0xFF];
    }
    return crc ^ 0xFFFFFFFF;
}

uint16_t ZmodemTransfer::calculate_crc16(const std::vector<unsigned char>& data) {
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

void ZmodemTransfer::cancel() {
    if (active_) {
        active_ = false;
        std::vector<unsigned char> data(4, 0);
        send_frame(FrameType::ZCAN, data);
    }
}

} // namespace filetransfer
