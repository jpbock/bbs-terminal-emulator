#pragma once

#include <string>
#include <vector>
#include <memory>
#include <chrono>

namespace dialing {

struct DialEntry {
    std::string name;
    std::string host;
    int port = 23;  // Default telnet port
    std::string username;
    std::string password;
    std::string terminal_type = "ANSI";
    bool auto_login = false;
    std::string comments;
    std::chrono::system_clock::time_point last_call;
    int times_called = 0;
};

class Phonebook {
public:
    Phonebook();
    ~Phonebook();

    void add_entry(const DialEntry& entry);
    void update_entry(size_t index, const DialEntry& entry);
    void remove_entry(size_t index);
    void clear();

    const DialEntry& get_entry(size_t index) const;
    size_t get_entry_count() const;
    const std::vector<DialEntry>& get_all_entries() const { return entries_; }

    // Search
    std::vector<size_t> search_by_name(const std::string& pattern) const;
    std::vector<size_t> search_by_host(const std::string& pattern) const;

    // Persistence
    bool load_from_file(const std::string& filename);
    bool save_to_file(const std::string& filename) const;

    // Statistics
    void log_connection(size_t index);
    int get_times_called(size_t index) const;

private:
    std::vector<DialEntry> entries_;
};

} // namespace dialing
