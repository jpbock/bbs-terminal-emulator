#include "settings.h"
#include <fstream>
#include <json/json.h>

namespace config {

Settings::Settings() {
}

Settings::~Settings() {
}

bool Settings::load_from_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;

    try {
        Json::Value root;
        file >> root;

        // Window
        if (root.isMember("window")) {
            window_width_ = root["window"]["width"].asInt();
            window_height_ = root["window"]["height"].asInt();
            font_name_ = root["window"]["font"].asString();
            font_size_ = root["window"]["font_size"].asInt();
        }

        // Terminal
        if (root.isMember("terminal")) {
            terminal_width_ = root["terminal"]["width"].asInt();
            terminal_height_ = root["terminal"]["height"].asInt();
            terminal_type_ = root["terminal"]["type"].asString();
        }

        // Colors
        if (root.isMember("colors")) {
            fg_color_ = root["colors"]["foreground"].asUInt();
            bg_color_ = root["colors"]["background"].asUInt();
        }

        // Features
        if (root.isMember("features")) {
            enable_sound_ = root["features"]["sound"].asBool();
            enable_rip_ = root["features"]["rip"].asBool();
            enable_logging_ = root["features"]["logging"].asBool();
        }

        return true;
    } catch (...) {
        return false;
    }
}

bool Settings::save_to_file(const std::string& filename) const {
    try {
        Json::Value root;

        // Window
        root["window"]["width"] = window_width_;
        root["window"]["height"] = window_height_;
        root["window"]["font"] = font_name_;
        root["window"]["font_size"] = font_size_;

        // Terminal
        root["terminal"]["width"] = terminal_width_;
        root["terminal"]["height"] = terminal_height_;
        root["terminal"]["type"] = terminal_type_;

        // Colors
        root["colors"]["foreground"] = fg_color_;
        root["colors"]["background"] = bg_color_;

        // Features
        root["features"]["sound"] = enable_sound_;
        root["features"]["rip"] = enable_rip_;
        root["features"]["logging"] = enable_logging_;

        std::ofstream file(filename);
        file << root.toStyledString();
        return true;
    } catch (...) {
        return false;
    }
}

} // namespace config
