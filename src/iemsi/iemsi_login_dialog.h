#pragma once

#include <string>
#include <vector>
#include <functional>
#include "iemsi_handler.h"

namespace iemsi {

class IEMSILoginDialog {
public:
    using LoginCallback = std::function<void(const IEMSIUserProfile&)>;

    IEMSILoginDialog();
    ~IEMSILoginDialog();

    // Show login dialog
    bool show_dialog(const std::string& bbs_name = "");

    // Set callback for successful login
    void set_login_callback(LoginCallback cb) { login_callback_ = cb; }

    // Get login profile
    const IEMSIUserProfile& get_profile() const { return profile_; }

    // Profile management
    void save_profile_to_file(const std::string& filename);
    bool load_profile_from_file(const std::string& filename);

private:
    IEMSIUserProfile profile_;
    LoginCallback login_callback_;

    // Dialog helpers
    void initialize_profile();
    bool validate_profile() const;
};

} // namespace iemsi
