#include "terminal_view.h"
#include <d2d1.h>
#include <dwrite.h>
#include <windows.h>

#pragma comment(lib, "dwrite.lib")

namespace ui {

TerminalView::TerminalView(int width, int height)
    : width_(width), height_(height),
      cell_width_(8), cell_height_(16),
      ansi_interpreter_(std::make_unique<terminal::ANSIInterpreter>(width, height)),
      rip_interpreter_(std::make_unique<terminal::RIPInterpreter>(640, 350)),
      screen_buffer_(std::make_unique<terminal::ScreenBuffer>(width, height)) {
}

TerminalView::~TerminalView() {
    if (render_target_) {
        render_target_->Release();
    }
    if (dwrite_factory_) {
        dwrite_factory_->Release();
    }
    if (d2d_factory_) {
        d2d_factory_->Release();
    }
}

void TerminalView::initialize(HWND parent_hwnd) {
    parent_hwnd_ = parent_hwnd;
    setup_graphics();
}

void TerminalView::append_data(const std::vector<unsigned char>& data) {
    // Process through ANSI interpreter
    ansi_interpreter_->process_input(data);

    // Look for RIP sequences
    std::string str_data(data.begin(), data.end());
    rip_interpreter_->process_input(str_data);

    render();
}

void TerminalView::render() {
    if (render_target_) {
        render_target_->BeginDraw();
        render_text();
        render_graphics();
        render_target_->EndDraw();
    }
}

void TerminalView::set_dimensions(int width, int height) {
    width_ = width;
    height_ = height;
    if (ansi_interpreter_) ansi_interpreter_->set_window_size(width, height);
    if (screen_buffer_) screen_buffer_->resize(width, height);
}

std::string TerminalView::get_selected_text() const {
    return screen_buffer_->get_all_text();
}

void TerminalView::copy_to_clipboard() {
    std::string text = get_selected_text();
    if (!text.empty()) {
        size_t size = text.size() + 1;
        HGLOBAL hglobal = GlobalAlloc(GMEM_MOVEABLE, size);
        if (hglobal) {
            memcpy(GlobalLock(hglobal), text.c_str(), size);
            GlobalUnlock(hglobal);
            if (OpenClipboard(parent_hwnd_)) {
                EmptyClipboard();
                SetClipboardData(CF_TEXT, hglobal);
                CloseClipboard();
            }
        }
    }
}

void TerminalView::setup_graphics() {
    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &d2d_factory_);
    DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(&dwrite_factory_));
}

void TerminalView::render_text() {
    if (!render_target_) return;

    // Clear background
    render_target_->Clear(D2D1::ColorF(D2D1::ColorF::Black));

    // Draw each character from screen buffer
    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            auto cell = screen_buffer_->get_cell(x, y);
            if (cell.ch != ' ') {
                // TODO: Draw character with appropriate color and style
            }
        }
    }
}

void TerminalView::render_graphics() {
    // RIP graphics would be rendered here
}

} // namespace ui
