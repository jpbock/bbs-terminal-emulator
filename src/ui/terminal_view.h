#pragma once

#include <windows.h>
#include <d2d1.h>
#include "../terminal/ansi_interpreter.h"
#include "../terminal/rip_interpreter.h"
#include "../terminal/screen_buffer.h"
#include <vector>
#include <memory>

#pragma comment(lib, "d2d1.lib")

namespace ui {

class TerminalView {
public:
    TerminalView(int width = 80, int height = 25);
    ~TerminalView();

    void initialize(HWND parent_hwnd);
    void append_data(const std::vector<unsigned char>& data);
    void render();

    void set_dimensions(int width, int height);
    int get_width() const { return width_; }
    int get_height() const { return height_; }

    // Copy selection
    std::string get_selected_text() const;
    void copy_to_clipboard();

private:
    void setup_graphics();
    void render_text();
    void render_graphics();
    void on_paint();

    int width_, height_;
    int cell_width_, cell_height_;

    HWND parent_hwnd_ = nullptr;
    ID2D1Factory* d2d_factory_ = nullptr;
    ID2D1HwndRenderTarget* render_target_ = nullptr;
    IDWriteFactory* dwrite_factory_ = nullptr;

    std::unique_ptr<terminal::ANSIInterpreter> ansi_interpreter_;
    std::unique_ptr<terminal::RIPInterpreter> rip_interpreter_;
    std::unique_ptr<terminal::ScreenBuffer> screen_buffer_;
};

} // namespace ui
