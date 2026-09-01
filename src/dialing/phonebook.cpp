#include "phonebook.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <json/json.h>
#include <iostream>

namespace dialing {

Phonebook::Phonebook() {
}

Phonebook::~Phonebook() {
}

void Phonebook::add_entry(const DialEntry& entry) {
    entries_.push_back(entry);
}

void Phonebook::update_entry(size_t index, const DialEntry& entry) {
    if (index < entries_.size()) {
        entries_[index] = entry;
    }
}

void Phonebook::remove_entry(size_t index) {
    if (index < entries_.size()) {
        entries_.erase(entries_.begin() + index);
    }
}

void Phonebook::clear() {
    entries_.clear();
}

const DialEntry& Phonebook::get_entry(size_t index) const {
    static DialEntry empty;
    if (index < entries_.size()) {
        return entries_[index];
    }
    return empty;
}

size_t Phonebook::get_entry_count() const {
    return entries_.size();
}

std::vector<size_t> Phonebook::search_by_name(const std::string& pattern) const {
    std::vector<size_t> results;
    std::string lower_pattern = pattern;
    std::transform(lower_pattern.begin(), lower_pattern.end(), lower_pattern.begin(), ::tolower);

    for (size_t i = 0; i < entries_.size(); ++i) {
        std::string lower_name = entries_[i].name;
        std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);
        if (lower_name.find(lower_pattern) != std::string::npos) {
            results.push_back(i);
        }
    }
    return results;
}

std::vector<size_t> Phonebook::search_by_host(const std::string& pattern) const {
    std::vector<size_t> results;
    std::string lower_pattern = pattern;
    std::transform(lower_pattern.begin(), lower_pattern.end(), lower_pattern.begin(), ::tolower);

    for (size_t i = 0; i < entries_.size(); ++i) {
        std::string lower_host = entries_[i].host;
        std::transform(lower_host.begin(), lower_host.end(), lower_host.begin(), ::tolower);
        if (lower_host.find(lower_pattern) != std::string::npos) {
            results.push_back(i);
        }
    }
    return results;
}

bool Phonebook::load_from_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;

    try {
        Json::Value root;
        file >> root;

        entries_.clear();
        for (const auto& entry_json : root["entries"]) {
            DialEntry entry;
            entry.name = entry_json["name"].asString();
            entry.host = entry_json["host"].asString();
            entry.port = entry_json["port"].asInt();
            entry.username = entry_json["username"].asString();
            entry.password = entry_json["password"].asString();
            entry.terminal_type = entry_json["terminal_type"].asString();
            entry.auto_login = entry_json["auto_login"].asBool();
            entry.comments = entry_json["comments"].asString();
            entry.times_called = entry_json["times_called"].asInt();
            entries_.push_back(entry);
        }
        return true;
    } catch (...) {
        return false;
    }
}

bool Phonebook::save_to_file(const std::string& filename) const {
    try {
        Json::Value root;
        Json::Value entries_array = Json::arrayValue;

        for (const auto& entry : entries_) {
            Json::Value entry_json;
            entry_json["name"] = entry.name;
            entry_json["host"] = entry.host;
            entry_json["port"] = entry.port;
            entry_json["username"] = entry.username;
            entry_json["password"] = entry.password;
            entry_json["terminal_type"] = entry.terminal_type;
            entry_json["auto_login"] = entry.auto_login;
            entry_json["comments"] = entry.comments;
            entry_json["times_called"] = entry.times_called;
            entries_array.append(entry_json);
        }
        root["entries"] = entries_array;

        std::ofstream file(filename);
        file << root.toStyledString();
        return true;
    } catch (...) {
        return false;
    }
}

void Phonebook::log_connection(size_t index) {
    if (index < entries_.size()) {
        entries_[index].times_called++;
        entries_[index].last_call = std::chrono::system_clock::now();
    }
}

int Phonebook::get_times_called(size_t index) const {
    if (index < entries_.size()) {
        return entries_[index].times_called;
    }
    return 0;
}

} // namespace dialing
