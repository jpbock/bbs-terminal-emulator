#pragma once

#include <string>
#include <map>
#include <memory>

namespace config {

class Settings {
public:
    Settings();
    ~Settings();

    // Window settings
    void set_window_width(int width) { window_width_ = width; }
    void set_window_height(int height) { window_height_ = height; }
    void set_font_name(const std::string& font) { font_name_ = font; }
    void set_font_size(int size) { font_size_ = size; }

    int get_window_width() const { return window_width_; }
    int get_window_height() const { return window_height_; }
    const std::string& get_font_name() const { return font_name_; }
    int get_font_size() const { return font_size_; }

    // Terminal settings
    void set_terminal_width(int width) { terminal_width_ = width; }
    void set_terminal_height(int height) { terminal_height_ = height; }
    void set_terminal_type(const std::string& type) { terminal_type_ = type; }

    int get_terminal_width() const { return terminal_width_; }
    int get_terminal_height() const { return terminal_height_; }
    const std::string& get_terminal_type() const { return terminal_type_; }

    // Color settings
    void set_foreground_color(unsigned int color) { fg_color_ = color; }
    void set_background_color(unsigned int color) { bg_color_ = color; }
    unsigned int get_foreground_color() const { return fg_color_; }
    unsigned int get_background_color() const { return bg_color_; }

    // Features
    void set_enable_sound(bool enable) { enable_sound_ = enable; }
    void set_enable_rip(bool enable) { enable_rip_ = enable; }
    void set_enable_logging(bool enable) { enable_logging_ = enable; }

    bool is_sound_enabled() const { return enable_sound_; }
    bool is_rip_enabled() const { return enable_rip_; }
    bool is_logging_enabled() const { return enable_logging_; }

    // Persistence
    bool load_from_file(const std::string& filename);
    bool save_to_file(const std::string& filename) const;

private:
    // Window
    int window_width_ = 1024;
    int window_height_ = 768;
    std::string font_name_ = "Consolas";
    int font_size_ = 12;

    // Terminal
    int terminal_width_ = 80;
    int terminal_height_ = 25;
    std::string terminal_type_ = "ANSI";

    // Colors
    unsigned int fg_color_ = 0xFFFFFF;  // White
    unsigned int bg_color_ = 0x000000;  // Black

    // Features
    bool enable_sound_ = true;
    bool enable_rip_ = true;
    bool enable_logging_ = false;
};

} // namespace config
