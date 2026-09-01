#pragma once

#include <string>
#include <vector>
#include <functional>

namespace terminal {

struct Color {
    unsigned char r, g, b;
    Color(unsigned char r = 255, unsigned char g = 255, unsigned char b = 255)
        : r(r), g(g), b(b) {}
};

struct TextAttribute {
    Color foreground{255, 255, 255};
    Color background{0, 0, 0};
    bool bold = false;
    bool dim = false;
    bool italic = false;
    bool underline = false;
    bool blink = false;
    bool reverse = false;
    bool hidden = false;
    bool strikethrough = false;
};

struct CharacterCell {
    unsigned char ch = ' ';
    TextAttribute attr;
};

class ANSIInterpreter {
public:
    ANSIInterpreter(int width = 80, int height = 25);

    using OutputCallback = std::function<void(const CharacterCell&, int col, int row)>;
    using ClearCallback = std::function<void()>;
    using CursorCallback = std::function<void(int col, int row)>;

    void process_input(const std::vector<unsigned char>& data);
    void set_output_callback(OutputCallback cb) { output_callback_ = cb; }
    void set_clear_callback(ClearCallback cb) { clear_callback_ = cb; }
    void set_cursor_callback(CursorCallback cb) { cursor_callback_ = cb; }

    int get_cursor_x() const { return cursor_x_; }
    int get_cursor_y() const { return cursor_y_; }
    int get_width() const { return width_; }
    int get_height() const { return height_; }

    void set_window_size(int width, int height);
    void clear_screen();

private:
    void process_escape_sequence(const std::string& seq);
    void process_csi_sequence(const std::string& params, char command);
    void handle_sgr(const std::vector<int>& params);
    void reset_attributes();
    void output_char(unsigned char ch);

    int width_, height_;
    int cursor_x_ = 0, cursor_y_ = 0;
    TextAttribute current_attr_;
    std::string escape_buffer_;

    OutputCallback output_callback_;
    ClearCallback clear_callback_;
    CursorCallback cursor_callback_;

    // SGR state
    std::vector<int> saved_sgr_params_;
};

} // namespace terminal
