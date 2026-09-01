#include <iostream>
#include <windows.h>
#include "ui/main_window.h"
#include "dialing/phonebook.h"
#include "config/settings.h"

int main(int argc, char* argv[]) {
    try {
        // Initialize COM for Windows
        CoInitializeEx(nullptr, COINIT_MULTITHREADED);

        // Create and show main window
        ui::MainWindow main_window;
        
        if (!main_window.create()) {
            std::cerr << "Failed to create main window" << std::endl;
            return 1;
        }

        if (!main_window.show()) {
            std::cerr << "Failed to show main window" << std::endl;
            return 1;
        }

        // Run message loop
        int result = main_window.run();

        CoUninitialize();
        return result;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
}
