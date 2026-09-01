#pragma once

#include <windows.h>
#include <string>
#include <memory>
#include "terminal_view.h"
#include "../telnet/telnet_client.h"
#include "../dialing/phonebook.h"
#include "../config/settings.h"

namespace ui {

class MainWindow {
public:
    MainWindow();
    ~MainWindow();

    bool create();
    bool show();
    int run();
    void destroy();

    // Window properties
    void set_title(const std::string& title);
    void set_dimensions(int width, int height);
    HWND get_handle() const { return hwnd_; }

    // Terminal operations
    void connect_to_bbs(const dialing::DialEntry& entry);
    void disconnect();
    void send_input(const std::string& text);

    // UI Updates
    void update_status(const std::string& status);
    void show_phonebook_dialog();
    void show_settings_dialog();
    void show_about_dialog();

    // Callbacks from telnet client
    void on_telnet_connected(bool success);
    void on_telnet_data_received(const std::vector<unsigned char>& data);
    void on_telnet_disconnected();

private:
    static LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
    LRESULT handle_message(UINT msg, WPARAM wparam, LPARAM lparam);

    void create_menu();
    void create_controls();
    void setup_telnet_callbacks();

    HWND hwnd_ = nullptr;
    HWND status_bar_ = nullptr;
    HWND input_box_ = nullptr;

    std::unique_ptr<TerminalView> terminal_view_;
    std::unique_ptr<telnet::TelnetClient> telnet_client_;
    std::unique_ptr<dialing::Phonebook> phonebook_;
    std::unique_ptr<config::Settings> settings_;

    std::string window_title_ = "BBS Terminal Emulator";
    bool connected_ = false;
};

} // namespace ui
