#include "ansi_interpreter.h"
#include <cctype>
#include <regex>
#include <iostream>

namespace terminal {

ANSIInterpreter::ANSIInterpreter(int width, int height)
    : width_(width), height_(height) {
}

void ANSIInterpreter::process_input(const std::vector<unsigned char>& data) {
    for (unsigned char byte : data) {
        if (byte == 0x1B) {  // ESC
            escape_buffer_.clear();
            escape_buffer_ += (char)byte;
        } else if (!escape_buffer_.empty()) {
            escape_buffer_ += (char)byte;

            // Check if sequence is complete
            if ((byte >= 0x40 && byte <= 0x7E) || escape_buffer_.length() > 100) {
                process_escape_sequence(escape_buffer_);
                escape_buffer_.clear();
            }
        } else if (byte == '\n') {
            cursor_y_++;
            if (cursor_y_ >= height_) cursor_y_ = height_ - 1;
            if (cursor_callback_) cursor_callback_(cursor_x_, cursor_y_);
        } else if (byte == '\r') {
            cursor_x_ = 0;
            if (cursor_callback_) cursor_callback_(cursor_x_, cursor_y_);
        } else if (byte == '\b') {
            if (cursor_x_ > 0) cursor_x_--;
            if (cursor_callback_) cursor_callback_(cursor_x_, cursor_y_);
        } else if (byte >= 32 && byte < 127) {
            output_char(byte);
        }
    }
}

void ANSIInterpreter::process_escape_sequence(const std::string& seq) {
    if (seq.length() < 2 || seq[0] != 0x1B) return;

    if (seq[1] == '[') {  // CSI sequence
        size_t cmd_pos = seq.find_last_of(seq.back());
        if (cmd_pos > 2) {
            std::string params = seq.substr(2, cmd_pos - 2);
            char command = seq.back();
            process_csi_sequence(params, command);
        }
    }
}

void ANSIInterpreter::process_csi_sequence(const std::string& params, char command) {
    std::vector<int> param_list;
    std::string current_param;

    for (char c : params) {
        if (c == ';') {
            param_list.push_back(current_param.empty() ? 0 : std::stoi(current_param));
            current_param.clear();
        } else if (std::isdigit(c)) {
            current_param += c;
        }
    }
    if (!current_param.empty()) {
        param_list.push_back(std::stoi(current_param));
    }

    switch (command) {
        case 'H':  // Cursor Position
        case 'f': {
            int row = param_list.size() > 0 && param_list[0] > 0 ? param_list[0] - 1 : 0;
            int col = param_list.size() > 1 && param_list[1] > 0 ? param_list[1] - 1 : 0;
            cursor_x_ = col;
            cursor_y_ = row;
            if (cursor_callback_) cursor_callback_(cursor_x_, cursor_y_);
            break;
        }
        case 'A':  // Cursor Up
            cursor_y_ = std::max(0, cursor_y_ - (param_list.empty() ? 1 : param_list[0]));
            if (cursor_callback_) cursor_callback_(cursor_x_, cursor_y_);
            break;
        case 'B':  // Cursor Down
            cursor_y_ = std::min(height_ - 1, cursor_y_ + (param_list.empty() ? 1 : param_list[0]));
            if (cursor_callback_) cursor_callback_(cursor_x_, cursor_y_);
            break;
        case 'C':  // Cursor Forward
            cursor_x_ = std::min(width_ - 1, cursor_x_ + (param_list.empty() ? 1 : param_list[0]));
            if (cursor_callback_) cursor_callback_(cursor_x_, cursor_y_);
            break;
        case 'D':  // Cursor Back
            cursor_x_ = std::max(0, cursor_x_ - (param_list.empty() ? 1 : param_list[0]));
            if (cursor_callback_) cursor_callback_(cursor_x_, cursor_y_);
            break;
        case 'J':  // Erase in Display
            if (clear_callback_) clear_callback_();
            break;
        case 'K':  // Erase in Line
            // TODO: Implement line erase
            break;
        case 'm':  // Select Graphic Rendition
            handle_sgr(param_list);
            break;
    }
}

void ANSIInterpreter::handle_sgr(const std::vector<int>& params) {
    if (params.empty()) {
        reset_attributes();
        return;
    }

    for (int param : params) {
        switch (param) {
            case 0:  // Reset
                reset_attributes();
                break;
            case 1:  // Bold
                current_attr_.bold = true;
                break;
            case 2:  // Dim
                current_attr_.dim = true;
                break;
            case 3:  // Italic
                current_attr_.italic = true;
                break;
            case 4:  // Underline
                current_attr_.underline = true;
                break;
            case 5:  // Blink
                current_attr_.blink = true;
                break;
            case 7:  // Reverse
                current_attr_.reverse = true;
                break;
            case 8:  // Hidden
                current_attr_.hidden = true;
                break;
            case 9:  // Strikethrough
                current_attr_.strikethrough = true;
                break;
            case 30: current_attr_.foreground = Color(0, 0, 0); break;         // Black
            case 31: current_attr_.foreground = Color(255, 0, 0); break;       // Red
            case 32: current_attr_.foreground = Color(0, 255, 0); break;       // Green
            case 33: current_attr_.foreground = Color(255, 255, 0); break;     // Yellow
            case 34: current_attr_.foreground = Color(0, 0, 255); break;       // Blue
            case 35: current_attr_.foreground = Color(255, 0, 255); break;     // Magenta
            case 36: current_attr_.foreground = Color(0, 255, 255); break;     // Cyan
            case 37: current_attr_.foreground = Color(255, 255, 255); break;   // White
            case 40: current_attr_.background = Color(0, 0, 0); break;         // Black bg
            case 41: current_attr_.background = Color(255, 0, 0); break;       // Red bg
            case 42: current_attr_.background = Color(0, 255, 0); break;       // Green bg
            case 43: current_attr_.background = Color(255, 255, 0); break;     // Yellow bg
            case 44: current_attr_.background = Color(0, 0, 255); break;       // Blue bg
            case 45: current_attr_.background = Color(255, 0, 255); break;     // Magenta bg
            case 46: current_attr_.background = Color(0, 255, 255); break;     // Cyan bg
            case 47: current_attr_.background = Color(255, 255, 255); break;   // White bg
        }
    }
}

void ANSIInterpreter::reset_attributes() {
    current_attr_ = TextAttribute();
}

void ANSIInterpreter::output_char(unsigned char ch) {
    if (output_callback_) {
        output_callback_(CharacterCell{ch, current_attr_}, cursor_x_, cursor_y_);
    }
    cursor_x_++;
    if (cursor_x_ >= width_) {
        cursor_x_ = 0;
        cursor_y_++;
    }
    if (cursor_y_ >= height_) {
        cursor_y_ = height_ - 1;
    }
    if (cursor_callback_) {
        cursor_callback_(cursor_x_, cursor_y_);
    }
}

void ANSIInterpreter::set_window_size(int width, int height) {
    width_ = width;
    height_ = height;
}

void ANSIInterpreter::clear_screen() {
    cursor_x_ = 0;
    cursor_y_ = 0;
    if (clear_callback_) {
        clear_callback_();
    }
}

} // namespace terminal
