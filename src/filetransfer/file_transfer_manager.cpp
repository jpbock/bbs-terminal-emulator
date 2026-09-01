#include "file_transfer_manager.h"
#include "xmodem_transfer.h"
#include "ymodem_transfer.h"
#include "zmodem_transfer.h"
#include <memory>

namespace filetransfer {

FileTransferManager::FileTransferManager() {
}

FileTransferManager::~FileTransferManager() {
}

bool FileTransferManager::start_send(ProtocolType protocol, const std::vector<std::string>& files) {
    if (state_ != TransferState::IDLE) {
        report_error("Transfer already in progress");
        return false;
    }

    protocol_ = protocol;
    files_.clear();

    for (const auto& file : files) {
        FileInfo info;
        info.full_path = file;
        // Parse filename from path
        size_t last_slash = file.find_last_of("\\/");
        if (last_slash != std::string::npos) {
            info.filename = file.substr(last_slash + 1);
        } else {
            info.filename = file;
        }
        files_.push_back(info);
    }

    set_state(TransferState::INITIATING);

    switch (protocol) {
        case ProtocolType::XMODEM:
        case ProtocolType::XMODEM_1K: {
            auto xmodem = std::make_unique<XmodemTransfer>(protocol == ProtocolType::XMODEM_1K);
            if (files.size() > 0) {
                xmodem->set_send_callback(send_data_callback_);
                xmodem->set_progress_callback([this](uint32_t bytes, uint32_t percent) {
                    if (progress_callback_ && !files_.empty()) {
                        files_[0].bytes_transferred = bytes;
                        progress_callback_(files_[0], percent);
                    }
                });
                xmodem->set_complete_callback([this](bool success) {
                    if (success) {
                        set_state(TransferState::COMPLETING);
                    } else {
                        report_error("Xmodem transfer failed");
                    }
                });
                xmodem->start_send(files[0]);
                set_state(TransferState::TRANSFERRING);
                return true;
            }
            break;
        }
        case ProtocolType::YMODEM:
        case ProtocolType::YMODEM_G: {
            auto ymodem = std::make_unique<YmodemTransfer>(protocol == ProtocolType::YMODEM_G);
            ymodem->set_send_callback(send_data_callback_);
            ymodem->set_progress_callback([this](uint32_t bytes, uint32_t percent) {
                if (progress_callback_ && current_file_index_ < files_.size()) {
                    files_[current_file_index_].bytes_transferred = bytes;
                    progress_callback_(files_[current_file_index_], percent);
                }
            });
            ymodem->set_complete_callback([this](bool success) {
                if (success) {
                    set_state(TransferState::COMPLETING);
                } else {
                    report_error("Ymodem transfer failed");
                }
            });
            ymodem->start_send(files);
            set_state(TransferState::TRANSFERRING);
            return true;
        }
        case ProtocolType::ZMODEM: {
            auto zmodem = std::make_unique<ZmodemTransfer>();
            zmodem->set_send_callback(send_data_callback_);
            zmodem->set_progress_callback([this](uint32_t bytes, uint32_t percent) {
                if (progress_callback_ && current_file_index_ < files_.size()) {
                    files_[current_file_index_].bytes_transferred = bytes;
                    progress_callback_(files_[current_file_index_], percent);
                }
            });
            zmodem->set_complete_callback([this](bool success) {
                if (success) {
                    set_state(TransferState::COMPLETING);
                } else {
                    report_error("Zmodem transfer failed");
                }
            });
            zmodem->start_send(files);
            set_state(TransferState::TRANSFERRING);
            return true;
        }
    }

    report_error("Unsupported protocol");
    return false;
}

bool FileTransferManager::start_receive(ProtocolType protocol, const std::string& save_directory) {
    if (state_ != TransferState::IDLE) {
        report_error("Transfer already in progress");
        return false;
    }

    protocol_ = protocol;
    receive_directory_ = save_directory;

    set_state(TransferState::INITIATING);

    switch (protocol) {
        case ProtocolType::XMODEM:
        case ProtocolType::XMODEM_1K: {
            auto xmodem = std::make_unique<XmodemTransfer>(protocol == ProtocolType::XMODEM_1K);
            xmodem->set_send_callback(send_data_callback_);
            xmodem->start_receive(save_directory);
            set_state(TransferState::TRANSFERRING);
            return true;
        }
        case ProtocolType::YMODEM:
        case ProtocolType::YMODEM_G: {
            auto ymodem = std::make_unique<YmodemTransfer>(protocol == ProtocolType::YMODEM_G);
            ymodem->set_send_callback(send_data_callback_);
            ymodem->start_receive(save_directory);
            set_state(TransferState::TRANSFERRING);
            return true;
        }
        case ProtocolType::ZMODEM: {
            auto zmodem = std::make_unique<ZmodemTransfer>();
            zmodem->set_send_callback(send_data_callback_);
            zmodem->start_receive(save_directory);
            set_state(TransferState::TRANSFERRING);
            return true;
        }
    }

    report_error("Unsupported protocol");
    return false;
}

void FileTransferManager::process_data(const std::vector<unsigned char>& data) {
    if (state_ != TransferState::TRANSFERRING) {
        return;
    }
    // Data processing delegated to protocol-specific handlers
}

void FileTransferManager::cancel_transfer() {
    if (state_ == TransferState::TRANSFERRING) {
        set_state(TransferState::CANCELLED);
    }
}

void FileTransferManager::set_state(TransferState state) {
    state_ = state;
    if (state_callback_) {
        state_callback_(state);
    }
}

void FileTransferManager::report_error(const std::string& error) {
    set_state(TransferState::ERROR);
    if (error_callback_) {
        error_callback_(error);
    }
}

} // namespace filetransfer
