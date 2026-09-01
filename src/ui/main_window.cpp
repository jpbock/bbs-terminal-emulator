#include "main_window.h"
#include <commctrl.h>
#include <iostream>
#include <sstream>

#pragma comment(lib, "comctl32.lib")

namespace ui {

static MainWindow* g_main_window = nullptr;

MainWindow::MainWindow()
    : terminal_view_(std::make_unique<TerminalView>()),
      telnet_client_(std::make_unique<telnet::TelnetClient>()),
      phonebook_(std::make_unique<dialing::Phonebook>()),
      settings_(std::make_unique<config::Settings>()) {
    g_main_window = this;
}

MainWindow::~MainWindow() {
    destroy();
}

bool MainWindow::create() {
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = wnd_proc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"BBSTerminalWindow";

    if (!RegisterClassEx(&wc)) {
        return false;
    }

    hwnd_ = CreateWindowEx(
        0,
        L"BBSTerminalWindow",
        L"BBS Terminal Emulator",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        1024, 768,
        nullptr,
        nullptr,
        GetModuleHandle(nullptr),
        this
    );

    if (!hwnd_) {
        return false;
    }

    create_menu();
    create_controls();
    setup_telnet_callbacks();

    return true;
}

bool MainWindow::show() {
    if (!hwnd_) return false;
    ShowWindow(hwnd_, SW_SHOW);
    UpdateWindow(hwnd_);
    return true;
}

int MainWindow::run() {
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}

void MainWindow::destroy() {
    if (hwnd_) {
        DestroyWindow(hwnd_);
        hwnd_ = nullptr;
    }
}

void MainWindow::set_title(const std::string& title) {
    window_title_ = title;
    if (hwnd_) {
        int len = MultiByteToWideChar(CP_UTF8, 0, title.c_str(), -1, nullptr, 0);
        std::wstring wtitle(len, 0);
        MultiByteToWideChar(CP_UTF8, 0, title.c_str(), -1, &wtitle[0], len);
        SetWindowText(hwnd_, wtitle.c_str());
    }
}

void MainWindow::set_dimensions(int width, int height) {
    if (hwnd_) {
        SetWindowPos(hwnd_, nullptr, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
    }
}

void MainWindow::connect_to_bbs(const dialing::DialEntry& entry) {
    if (connected_) {
        disconnect();
    }

    update_status("Connecting to " + entry.name + "...");
    telnet_client_->connect(entry.host, entry.port);
}

void MainWindow::disconnect() {
    if (telnet_client_->is_connected()) {
        telnet_client_->disconnect();
        connected_ = false;
        update_status("Disconnected");
    }
}

void MainWindow::send_input(const std::string& text) {
    if (telnet_client_->is_connected()) {
        telnet_client_->send_text(text + "\r\n");
    }
}

void MainWindow::update_status(const std::string& status) {
    if (status_bar_) {
        int len = MultiByteToWideChar(CP_UTF8, 0, status.c_str(), -1, nullptr, 0);
        std::wstring wstatus(len, 0);
        MultiByteToWideChar(CP_UTF8, 0, status.c_str(), -1, &wstatus[0], len);
        SendMessage(status_bar_, SB_SETTEXT, (WPARAM)0, (LPARAM)wstatus.c_str());
    }
}

void MainWindow::show_phonebook_dialog() {
    // TODO: Implement phonebook dialog
}

void MainWindow::show_settings_dialog() {
    // TODO: Implement settings dialog
}

void MainWindow::show_about_dialog() {
    MessageBox(hwnd_,
        L"BBS Terminal Emulator v1.0\n\n"
        L"A modern telnet BBS terminal with ANSI and RIP 1.54 support\n\n"
        L"© 2026",
        L"About BBS Terminal Emulator",
        MB_ICONINFORMATION | MB_OK);
}

void MainWindow::create_menu() {
    HMENU hmenu = CreateMenu();
    HMENU file_menu = CreatePopupMenu();
    HMENU edit_menu = CreatePopupMenu();
    HMENU tools_menu = CreatePopupMenu();
    HMENU help_menu = CreatePopupMenu();

    AppendMenu(file_menu, MFT_STRING, 1001, L"&Phonebook");
    AppendMenu(file_menu, MFT_STRING, 1002, L"&Disconnect");
    AppendMenu(file_menu, MFT_SEPARATOR, 0, nullptr);
    AppendMenu(file_menu, MFT_STRING, 1003, L"E&xit");

    AppendMenu(edit_menu, MFT_STRING, 2001, L"&Copy");
    AppendMenu(edit_menu, MFT_STRING, 2002, L"&Paste");
    AppendMenu(edit_menu, MFT_STRING, 2003, L"&Select All");

    AppendMenu(tools_menu, MFT_STRING, 3001, L"&Settings");
    AppendMenu(tools_menu, MFT_STRING, 3002, L"&Capture Log");

    AppendMenu(help_menu, MFT_STRING, 4001, L"&About");

    AppendMenu(hmenu, MFT_POPUP, (UINT_PTR)file_menu, L"&File");
    AppendMenu(hmenu, MFT_POPUP, (UINT_PTR)edit_menu, L"&Edit");
    AppendMenu(hmenu, MFT_POPUP, (UINT_PTR)tools_menu, L"&Tools");
    AppendMenu(hmenu, MFT_POPUP, (UINT_PTR)help_menu, L"&Help");

    SetMenu(hwnd_, hmenu);
}

void MainWindow::create_controls() {
    // Create status bar
    status_bar_ = CreateWindowEx(
        0,
        STATUSCLASSNAME,
        L"Ready",
        WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
        0, 0, 0, 0,
        hwnd_,
        (HMENU)1,
        GetModuleHandle(nullptr),
        nullptr
    );

    // Create input box
    input_box_ = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        L"EDIT",
        L"",
        WS_CHILD | WS_VISIBLE | WS_HSCROLL | ES_MULTILINE,
        10, 10, 1000, 700,
        hwnd_,
        (HMENU)2,
        GetModuleHandle(nullptr),
        nullptr
    );
}

void MainWindow::setup_telnet_callbacks() {
    telnet_client_->set_connect_callback(
        [this](bool success) { on_telnet_connected(success); }
    );
    telnet_client_->set_data_callback(
        [this](const std::vector<unsigned char>& data) { on_telnet_data_received(data); }
    );
    telnet_client_->set_disconnect_callback(
        [this]() { on_telnet_disconnected(); }
    );
}

void MainWindow::on_telnet_connected(bool success) {
    if (success) {
        connected_ = true;
        update_status("Connected");
    } else {
        update_status("Connection failed");
    }
}

void MainWindow::on_telnet_data_received(const std::vector<unsigned char>& data) {
    if (terminal_view_) {
        terminal_view_->append_data(data);
    }
}

void MainWindow::on_telnet_disconnected() {
    connected_ = false;
    update_status("Disconnected");
}

LRESULT CALLBACK MainWindow::wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    MainWindow* pThis = nullptr;

    if (msg == WM_CREATE) {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lparam);
        pThis = reinterpret_cast<MainWindow*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
    } else {
        pThis = reinterpret_cast<MainWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (pThis) {
        return pThis->handle_message(msg, wparam, lparam);
    }

    return DefWindowProc(hwnd, msg, wparam, lparam);
}

LRESULT MainWindow::handle_message(UINT msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
        case WM_COMMAND: {
            int cmd = LOWORD(wparam);
            switch (cmd) {
                case 1001: show_phonebook_dialog(); break;
                case 1002: disconnect(); break;
                case 1003: PostQuitMessage(0); break;
                case 3001: show_settings_dialog(); break;
                case 4001: show_about_dialog(); break;
            }
            break;
        }
        case WM_SIZE:
            if (status_bar_) SendMessage(status_bar_, WM_SIZE, 0, 0);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd_, msg, wparam, lparam);
    }
    return 0;
}

} // namespace ui
