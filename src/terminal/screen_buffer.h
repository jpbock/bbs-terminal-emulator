#pragma once

#include "ansi_interpreter.h"
#include <vector>
#include <string>

namespace terminal {

class ScreenBuffer {
public:
    ScreenBuffer(int width = 80, int height = 25);

    void put_char(unsigned char ch, int col, int row, const TextAttribute& attr);
    void put_string(const std::string& str, int col, int row, const TextAttribute& attr);
    void clear();
    void scroll_up(int lines = 1);
    void scroll_down(int lines = 1);

    CharacterCell get_cell(int col, int row) const;
    std::string get_line(int row) const;
    std::string get_all_text() const;

    int get_width() const { return width_; }
    int get_height() const { return height_; }

    const std::vector<std::vector<CharacterCell>>& get_buffer() const { return buffer_; }

    void resize(int width, int height);

private:
    int width_, height_;
    std::vector<std::vector<CharacterCell>> buffer_;
};

} // namespace terminal
