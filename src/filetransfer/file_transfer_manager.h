#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <functional>

namespace filetransfer {

enum class ProtocolType {
    XMODEM,
    XMODEM_1K,
    YMODEM,
    YMODEM_G,
    ZMODEM
};

enum class TransferState {
    IDLE,
    INITIATING,
    TRANSFERRING,
    COMPLETING,
    ERROR,
    CANCELLED
};

struct FileInfo {
    std::string filename;
    uint32_t filesize = 0;
    uint32_t modification_time = 0;
    std::string full_path;
    uint32_t bytes_transferred = 0;
};

class FileTransferManager {
public:
    FileTransferManager();
    ~FileTransferManager();

    using DataCallback = std::function<void(const std::vector<unsigned char>&)>;
    using ProgressCallback = std::function<void(const FileInfo&, int percent)>;
    using StateCallback = std::function<void(TransferState)>;
    using ErrorCallback = std::function<void(const std::string&)>;
    using FileListCallback = std::function<void(const std::vector<FileInfo>&)>;

    // Initiate transfers
    bool start_send(ProtocolType protocol, const std::vector<std::string>& files);
    bool start_receive(ProtocolType protocol, const std::string& save_directory);
    void cancel_transfer();

    // Process incoming data
    void process_data(const std::vector<unsigned char>& data);

    // Callbacks
    void set_send_data_callback(DataCallback cb) { send_data_callback_ = cb; }
    void set_progress_callback(ProgressCallback cb) { progress_callback_ = cb; }
    void set_state_callback(StateCallback cb) { state_callback_ = cb; }
    void set_error_callback(ErrorCallback cb) { error_callback_ = cb; }
    void set_file_list_callback(FileListCallback cb) { file_list_callback_ = cb; }

    // State queries
    TransferState get_state() const { return state_; }
    ProtocolType get_protocol() const { return protocol_; }
    const FileInfo& get_current_file() const { return current_file_; }

private:
    TransferState state_ = TransferState::IDLE;
    ProtocolType protocol_;

    std::vector<FileInfo> files_;
    FileInfo current_file_;
    size_t current_file_index_ = 0;

    std::string receive_directory_;
    std::vector<unsigned char> receive_buffer_;

    DataCallback send_data_callback_;
    ProgressCallback progress_callback_;
    StateCallback state_callback_;
    ErrorCallback error_callback_;
    FileListCallback file_list_callback_;

    void set_state(TransferState state);
    void report_error(const std::string& error);
};

} // namespace filetransfer
