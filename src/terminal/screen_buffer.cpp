#include "screen_buffer.h"
#include <algorithm>

namespace terminal {

ScreenBuffer::ScreenBuffer(int width, int height)
    : width_(width), height_(height) {
    buffer_.resize(height, std::vector<CharacterCell>(width));
}

void ScreenBuffer::put_char(unsigned char ch, int col, int row, const TextAttribute& attr) {
    if (col >= 0 && col < width_ && row >= 0 && row < height_) {
        buffer_[row][col].ch = ch;
        buffer_[row][col].attr = attr;
    }
}

void ScreenBuffer::put_string(const std::string& str, int col, int row, const TextAttribute& attr) {
    for (size_t i = 0; i < str.length(); ++i) {
        if (col + i >= width_) break;
        put_char(str[i], col + i, row, attr);
    }
}

void ScreenBuffer::clear() {
    for (auto& row : buffer_) {
        for (auto& cell : row) {
            cell.ch = ' ';
            cell.attr = TextAttribute();
        }
    }
}

void ScreenBuffer::scroll_up(int lines) {
    lines = std::min(lines, height_);
    for (int i = 0; i < height_ - lines; ++i) {
        buffer_[i] = buffer_[i + lines];
    }
    for (int i = height_ - lines; i < height_; ++i) {
        for (auto& cell : buffer_[i]) {
            cell.ch = ' ';
            cell.attr = TextAttribute();
        }
    }
}

void ScreenBuffer::scroll_down(int lines) {
    lines = std::min(lines, height_);
    for (int i = height_ - 1; i >= lines; --i) {
        buffer_[i] = buffer_[i - lines];
    }
    for (int i = 0; i < lines; ++i) {
        for (auto& cell : buffer_[i]) {
            cell.ch = ' ';
            cell.attr = TextAttribute();
        }
    }
}

CharacterCell ScreenBuffer::get_cell(int col, int row) const {
    if (col >= 0 && col < width_ && row >= 0 && row < height_) {
        return buffer_[row][col];
    }
    return CharacterCell();
}

std::string ScreenBuffer::get_line(int row) const {
    std::string result;
    if (row >= 0 && row < height_) {
        for (const auto& cell : buffer_[row]) {
            result += cell.ch;
        }
    }
    return result;
}

std::string ScreenBuffer::get_all_text() const {
    std::string result;
    for (const auto& row : buffer_) {
        for (const auto& cell : row) {
            result += cell.ch;
        }
        result += '\n';
    }
    return result;
}

void ScreenBuffer::resize(int width, int height) {
    width_ = width;
    height_ = height;
    buffer_.clear();
    buffer_.resize(height, std::vector<CharacterCell>(width));
}

} // namespace terminal
