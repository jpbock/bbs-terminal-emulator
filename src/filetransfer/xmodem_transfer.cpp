#include "xmodem_transfer.h"
#include <fstream>
#include <algorithm>
#include <windows.h>

namespace filetransfer {

XmodemTransfer::XmodemTransfer(bool use_1k)
    : use_1k_blocks_(use_1k) {
}

XmodemTransfer::~XmodemTransfer() {
    cancel();
}

bool XmodemTransfer::start_send(const std::string& filename) {
    // Load file
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return false;
    }

    file_size_ = (uint32_t)file.tellg();
    file.seekg(0, std::ios::beg);

    file_buffer_.resize(file_size_);
    file.read((char*)file_buffer_.data(), file_size_);
    file.close();

    filename_ = filename;
    sending_ = true;
    active_ = true;
    block_number_ = 1;
    file_offset_ = 0;

    // Wait for receiver to send NAK
    return true;
}

bool XmodemTransfer::start_receive(const std::string& save_path) {
    save_path_ = save_path;
    sending_ = false;
    active_ = true;
    block_number_ = 1;
    file_offset_ = 0;
    file_size_ = 0;

    send_start_signal();
    return true;
}

void XmodemTransfer::send_start_signal() {
    // Send NAK to initiate transfer
    std::vector<unsigned char> nak{NAK};
    if (send_callback_) {
        send_callback_(nak);
    }
}

void XmodemTransfer::process_receive_data(const std::vector<unsigned char>& data) {
    if (!active_) return;

    for (unsigned char byte : data) {
        if (sending_) {
            if (byte == NAK) {
                send_block();
            } else if (byte == ACK) {
                file_offset_ += current_block_.size();
                block_number_++;
                if (file_offset_ >= file_size_) {
                    send_eot();
                    active_ = false;
                    if (complete_callback_) complete_callback_(true);
                } else {
                    send_block();
                }
            } else if (byte == CAN) {
                active_ = false;
                if (complete_callback_) complete_callback_(false);
            }
        } else {
            // Receiving mode
            if (byte == SOH || byte == STX) {
                // Block header received - need to read block
                unsigned char header = byte;
                uint32_t block_size = (header == STX) ? 1024 : 128;
                // TODO: Implement block reception
            }
        }
    }
}

void XmodemTransfer::send_block() {
    if (file_offset_ >= file_size_) {
        return;
    }

    uint32_t block_size = use_1k_blocks_ ? 1024 : 128;
    uint32_t bytes_to_read = std::min(block_size, file_size_ - file_offset_);

    std::vector<unsigned char> block_data(
        file_buffer_.begin() + file_offset_,
        file_buffer_.begin() + file_offset_ + bytes_to_read
    );

    // Pad with SUB if necessary
    if (block_data.size() < block_size) {
        block_data.resize(block_size, SUB);
    }

    current_block_ = create_block(block_number_, block_data);

    if (send_callback_) {
        send_callback_(current_block_);
    }

    if (progress_callback_) {
        uint32_t percent = (file_offset_ * 100) / file_size_;
        progress_callback_(file_offset_, percent);
    }
}

void XmodemTransfer::send_eot() {
    std::vector<unsigned char> eot{EOT};
    if (send_callback_) {
        send_callback_(eot);
    }
}

std::vector<unsigned char> XmodemTransfer::create_block(
    uint32_t block_num,
    const std::vector<unsigned char>& data) {

    std::vector<unsigned char> block;
    unsigned char header = use_1k_blocks_ ? STX : SOH;

    block.push_back(header);
    block.push_back((unsigned char)(block_num & 0xFF));
    block.push_back((unsigned char)(~(block_num & 0xFF)));

    block.insert(block.end(), data.begin(), data.end());

    uint8_t checksum = calculate_checksum(data);
    block.push_back(checksum);

    return block;
}

uint8_t XmodemTransfer::calculate_checksum(const std::vector<unsigned char>& data) {
    uint8_t sum = 0;
    for (unsigned char byte : data) {
        sum = (sum + byte) & 0xFF;
    }
    return sum;
}

bool XmodemTransfer::verify_block(const std::vector<unsigned char>& data) {
    if (data.size() < 4) return false;

    unsigned char header = data[0];
    if (header != SOH && header != STX) return false;

    unsigned char block_num = data[1];
    unsigned char block_num_inv = data[2];
    if ((block_num + block_num_inv) != 0xFF) return false;

    return true;
}

void XmodemTransfer::cancel() {
    if (active_) {
        active_ = false;
        std::vector<unsigned char> cancel{CAN, CAN, CAN};
        if (send_callback_) {
            send_callback_(cancel);
        }
    }
}

} // namespace filetransfer
