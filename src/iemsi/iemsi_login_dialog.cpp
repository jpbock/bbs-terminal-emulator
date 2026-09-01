#include "iemsi_login_dialog.h"
#include <fstream>
#include <json/json.h>
#include <windows.h>

namespace iemsi {

IEMSILoginDialog::IEMSILoginDialog() {
    initialize_profile();
}

IEMSILoginDialog::~IEMSILoginDialog() {
}

bool IEMSILoginDialog::show_dialog(const std::string& bbs_name) {
    // In a full implementation, this would show a Windows dialog
    // For now, we provide the infrastructure for dialog handling

    profile_.bbs_name = bbs_name;

    // Attempt to load existing profile
    std::string profile_file = std::string(getenv("APPDATA")) + "\\BBSTerminal\\profiles.json";
    load_profile_from_file(profile_file);

    return validate_profile();
}

void IEMSILoginDialog::initialize_profile() {
    profile_.user_name = "";
    profile_.real_name = "";
    profile_.alias = "";
    profile_.email_address = "";
    profile_.phone_number = "";
    profile_.terminal_type = "ANSI";
    profile_.terminal_width = 80;
    profile_.terminal_height = 25;
    profile_.supports_color = true;
    profile_.supports_graphics = false;
    profile_.new_user = false;
}

bool IEMSILoginDialog::validate_profile() const {
    // At minimum, need username
    return !profile_.user_name.empty();
}

void IEMSILoginDialog::save_profile_to_file(const std::string& filename) {
    try {
        Json::Value root;

        root["user_name"] = profile_.user_name;
        root["real_name"] = profile_.real_name;
        root["alias"] = profile_.alias;
        root["email_address"] = profile_.email_address;
        root["phone_number"] = profile_.phone_number;
        root["terminal_type"] = profile_.terminal_type;
        root["terminal_width"] = profile_.terminal_width;
        root["terminal_height"] = profile_.terminal_height;
        root["supports_color"] = profile_.supports_color;
        root["supports_graphics"] = profile_.supports_graphics;
        root["baud_rate"] = profile_.baud_rate;
        root["call_count"] = profile_.call_count;
        root["new_user"] = profile_.new_user;
        root["deleted_user"] = profile_.deleted_user;

        // Capabilities
        root["capabilities"]["terminal"] = profile_.capabilities.supports_terminal;
        root["capabilities"]["file_transfer"] = profile_.capabilities.supports_file_transfer;
        root["capabilities"]["chat"] = profile_.capabilities.supports_chat;
        root["capabilities"]["file_request"] = profile_.capabilities.supports_file_request;
        root["capabilities"]["voice"] = profile_.capabilities.supports_voice;
        root["capabilities"]["newsreader"] = profile_.capabilities.supports_newsreader;
        root["capabilities"]["archiver"] = profile_.capabilities.supports_archiver;
        root["capabilities"]["modem"] = profile_.capabilities.supports_modem;

        std::ofstream file(filename);
        file << root.toStyledString();
    } catch (...) {
        // Handle file write errors
    }
}

bool IEMSILoginDialog::load_profile_from_file(const std::string& filename) {
    try {
        std::ifstream file(filename);
        if (!file.is_open()) return false;

        Json::Value root;
        file >> root;

        profile_.user_name = root["user_name"].asString();
        profile_.real_name = root["real_name"].asString();
        profile_.alias = root["alias"].asString();
        profile_.email_address = root["email_address"].asString();
        profile_.phone_number = root["phone_number"].asString();
        profile_.terminal_type = root["terminal_type"].asString();
        profile_.terminal_width = root["terminal_width"].asUInt();
        profile_.terminal_height = root["terminal_height"].asUInt();
        profile_.supports_color = root["supports_color"].asBool();
        profile_.supports_graphics = root["supports_graphics"].asBool();
        profile_.baud_rate = root["baud_rate"].asUInt();
        profile_.call_count = root["call_count"].asUInt();
        profile_.new_user = root["new_user"].asBool();
        profile_.deleted_user = root["deleted_user"].asBool();

        // Capabilities
        if (root.isMember("capabilities")) {
            profile_.capabilities.supports_terminal = root["capabilities"]["terminal"].asBool();
            profile_.capabilities.supports_file_transfer = root["capabilities"]["file_transfer"].asBool();
            profile_.capabilities.supports_chat = root["capabilities"]["chat"].asBool();
            profile_.capabilities.supports_file_request = root["capabilities"]["file_request"].asBool();
            profile_.capabilities.supports_voice = root["capabilities"]["voice"].asBool();
            profile_.capabilities.supports_newsreader = root["capabilities"]["newsreader"].asBool();
            profile_.capabilities.supports_archiver = root["capabilities"]["archiver"].asBool();
            profile_.capabilities.supports_modem = root["capabilities"]["modem"].asBool();
        }

        return true;
    } catch (...) {
        return false;
    }
}

} // namespace iemsi
