#include "rip_interpreter.h"
#include <sstream>
#include <algorithm>
#include <cmath>

namespace terminal {

RIPInterpreter::RIPInterpreter(int width, int height)
    : width_(width), height_(height) {
}

void RIPInterpreter::process_input(const std::string& data) {
    // RIP sequences start with !|, end with |!
    size_t start = 0;
    while ((start = data.find("!|", start)) != std::string::npos) {
        size_t end = data.find("|!", start + 2);
        if (end == std::string::npos) break;

        std::string cmd = data.substr(start + 2, end - start - 2);
        parse_rip_command(cmd);
        start = end + 2;
    }
}

void RIPInterpreter::parse_rip_command(const std::string& cmd) {
    if (cmd.empty()) return;

    auto parts = split_command(cmd);
    if (parts.empty()) return;

    char command = parts[0][0];

    try {
        switch (command) {
            case 'Z':  // Reset
                current_color_ = 7;
                write_mode_ = 0;
                break;

            case 'E':  // Erase window
                erase_window();
                break;

            case 'P': {  // Set pixel
                if (parts.size() >= 4) {
                    int x = std::stoi(parts[1]);
                    int y = std::stoi(parts[2]);
                    int color = parse_color(parts[3]);
                    if (pixel_callback_) pixel_callback_(x, y, color);
                }
                break;
            }

            case 'L': {  // Draw line
                if (parts.size() >= 5) {
                    int x1 = std::stoi(parts[1]);
                    int y1 = std::stoi(parts[2]);
                    int x2 = std::stoi(parts[3]);
                    int y2 = std::stoi(parts[4]);
                    int color = parts.size() > 5 ? parse_color(parts[5]) : current_color_;
                    if (line_callback_) line_callback_(x1, y1, x2, y2, color);
                }
                break;
            }

            case 'R': {  // Rectangle
                if (parts.size() >= 5) {
                    int x1 = std::stoi(parts[1]);
                    int y1 = std::stoi(parts[2]);
                    int x2 = std::stoi(parts[3]);
                    int y2 = std::stoi(parts[4]);
                    int color = parts.size() > 5 ? parse_color(parts[5]) : current_color_;
                    if (rect_callback_) rect_callback_(x1, y1, x2, y2, color, false);
                }
                break;
            }

            case 'r': {  // Filled rectangle
                if (parts.size() >= 5) {
                    int x1 = std::stoi(parts[1]);
                    int y1 = std::stoi(parts[2]);
                    int x2 = std::stoi(parts[3]);
                    int y2 = std::stoi(parts[4]);
                    int color = parts.size() > 5 ? parse_color(parts[5]) : current_color_;
                    if (rect_callback_) rect_callback_(x1, y1, x2, y2, color, true);
                }
                break;
            }

            case 'C': {  // Circle
                if (parts.size() >= 4) {
                    int x = std::stoi(parts[1]);
                    int y = std::stoi(parts[2]);
                    int radius = std::stoi(parts[3]);
                    int color = parts.size() > 4 ? parse_color(parts[4]) : current_color_;
                    if (circle_callback_) circle_callback_(x, y, radius, color, false);
                }
                break;
            }

            case 'c': {  // Filled circle
                if (parts.size() >= 4) {
                    int x = std::stoi(parts[1]);
                    int y = std::stoi(parts[2]);
                    int radius = std::stoi(parts[3]);
                    int color = parts.size() > 4 ? parse_color(parts[4]) : current_color_;
                    if (circle_callback_) circle_callback_(x, y, radius, color, true);
                }
                break;
            }

            case 'T': {  // Draw text
                if (parts.size() >= 4) {
                    int x = std::stoi(parts[1]);
                    int y = std::stoi(parts[2]);
                    int color = parts.size() > 4 ? parse_color(parts[4]) : current_color_;
                    if (text_callback_) text_callback_(x, y, parts[3], color);
                }
                break;
            }

            case 'w': {  // Set write mode
                if (parts.size() >= 2) {
                    set_write_mode(std::stoi(parts[1]));
                }
                break;
            }

            case 'p': {  // Set palette
                if (parts.size() >= 5) {
                    int index = std::stoi(parts[1]);
                    int r = std::stoi(parts[2]);
                    int g = std::stoi(parts[3]);
                    int b = std::stoi(parts[4]);
                    set_palette(index, r, g, b);
                }
                break;
            }
        }
    } catch (...) {
        // Ignore parsing errors
    }
}

std::vector<std::string> RIPInterpreter::split_command(const std::string& cmd) {
    std::vector<std::string> result;
    std::stringstream ss(cmd);
    std::string item;
    while (std::getline(ss, item, ' ')) {
        if (!item.empty()) {
            result.push_back(item);
        }
    }
    return result;
}

int RIPInterpreter::parse_color(const std::string& color_str) {
    try {
        if (color_str.find(',') != std::string::npos) {
            // RGB format: "r,g,b"
            int r = std::stoi(color_str);
            size_t pos1 = color_str.find(',');
            int g = std::stoi(color_str.substr(pos1 + 1));
            size_t pos2 = color_str.find(',', pos1 + 1);
            int b = std::stoi(color_str.substr(pos2 + 1));
            return (r << 16) | (g << 8) | b;
        } else {
            // Palette index
            return std::stoi(color_str);
        }
    } catch (...) {
        return current_color_;
    }
}

void RIPInterpreter::erase_window() {
    // Erase graphics window
}

void RIPInterpreter::set_palette(int index, int r, int g, int b) {
    // Set palette entry (would require graphics backend integration)
}

void RIPInterpreter::set_write_mode(int mode) {
    write_mode_ = mode;
}

} // namespace terminal
