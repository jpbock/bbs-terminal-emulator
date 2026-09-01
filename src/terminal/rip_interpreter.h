#pragma once

#include <string>
#include <vector>
#include <functional>

namespace terminal {

// RIP 1.54 Graphics commands
enum class RIPCommand {
    RESET,
    ERASE_WINDOW,
    SET_PIXEL,
    DRAW_LINE,
    DRAW_FILLED_RECTANGLE,
    DRAW_RECTANGLE,
    DRAW_FILLED_POLYGON,
    DRAW_POLYGON,
    DRAW_CIRCLE,
    DRAW_FILLED_CIRCLE,
    DRAW_ARC,
    DRAW_FILLED_ARC,
    COPY_REGION,
    QUERY_GRAPHICS,
    QUERY_SCREEN,
    SET_PALETTE,
    SET_WRITE_MODE,
    DRAW_TEXT,
    LOAD_FONT,
    DRAW_POLYLINE
};

struct RIPPixel {
    int x, y;
    int color;
};

struct RIPLine {
    int x1, y1, x2, y2;
    int color;
    int style;
};

class RIPInterpreter {
public:
    RIPInterpreter(int width = 640, int height = 350);

    using PixelCallback = std::function<void(int, int, int)>;
    using LineCallback = std::function<void(int, int, int, int, int)>;
    using RectCallback = std::function<void(int, int, int, int, int, bool)>;
    using CircleCallback = std::function<void(int, int, int, int, bool)>;
    using TextCallback = std::function<void(int, int, const std::string&, int)>;

    void process_input(const std::string& data);

    void set_pixel_callback(PixelCallback cb) { pixel_callback_ = cb; }
    void set_line_callback(LineCallback cb) { line_callback_ = cb; }
    void set_rect_callback(RectCallback cb) { rect_callback_ = cb; }
    void set_circle_callback(CircleCallback cb) { circle_callback_ = cb; }
    void set_text_callback(TextCallback cb) { text_callback_ = cb; }

    void erase_window();
    void set_palette(int index, int r, int g, int b);
    void set_write_mode(int mode);

    int get_width() const { return width_; }
    int get_height() const { return height_; }

private:
    void parse_rip_command(const std::string& cmd);
    std::vector<std::string> split_command(const std::string& cmd);
    int parse_color(const std::string& color_str);

    int width_, height_;
    int current_color_ = 7;  // Default white
    int write_mode_ = 0;      // Normal

    // Callbacks
    PixelCallback pixel_callback_;
    LineCallback line_callback_;
    RectCallback rect_callback_;
    CircleCallback circle_callback_;
    TextCallback text_callback_;
};

} // namespace terminal
