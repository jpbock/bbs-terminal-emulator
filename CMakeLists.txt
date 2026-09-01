cmake_minimum_required(VERSION 3.20)
project(BBSTerminalEmulator VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(jsoncpp REQUIRED)

# Source files
set(SOURCES
    src/main.cpp
    
    # Telnet module
    src/telnet/telnet_client.cpp
    
    # Terminal module
    src/terminal/ansi_interpreter.cpp
    src/terminal/rip_interpreter.cpp
    src/terminal/screen_buffer.cpp
    
    # UI module
    src/ui/main_window.cpp
    src/ui/terminal_view.cpp
    
    # Dialing module
    src/dialing/phonebook.cpp
    
    # Config module
    src/config/settings.cpp
    
    # File Transfer module
    src/filetransfer/file_transfer_manager.cpp
    src/filetransfer/xmodem_transfer.cpp
    src/filetransfer/ymodem_transfer.cpp
    src/filetransfer/zmodem_transfer.cpp
    
    # IEMSI module
    src/iemsi/iemsi_handler.cpp
    src/iemsi/iemsi_login_dialog.cpp
)

# Header files
set(HEADERS
    src/telnet/telnet_client.h
    src/terminal/ansi_interpreter.h
    src/terminal/rip_interpreter.h
    src/terminal/screen_buffer.h
    src/ui/main_window.h
    src/ui/terminal_view.h
    src/dialing/phonebook.h
    src/config/settings.h
    src/filetransfer/file_transfer_manager.h
    src/filetransfer/xmodem_transfer.h
    src/filetransfer/ymodem_transfer.h
    src/filetransfer/zmodem_transfer.h
    src/iemsi/iemsi_handler.h
    src/iemsi/iemsi_login_dialog.h
)

# Create executable
add_executable(bbs-terminal ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(bbs-terminal PRIVATE
    jsoncpp_lib
    ws2_32              # Winsock2 for telnet
    winmm               # Windows multimedia
    comctl32            # Common controls
    shell32             # Shell API
    ole32               # COM
    oleaut32            # OLE Automation
    d2d1                # Direct2D
    dwrite              # DirectWrite
    user32              # Windows User API
)

# Include directories
target_include_directories(bbs-terminal PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Compiler options
if(MSVC)
    target_compile_options(bbs-terminal PRIVATE
        /W4                 # Warning level 4
        /WX                 # Treat warnings as errors
        /permissive-        # Strict conformance
        /std:c++latest      # Latest C++ standard
    )
    
    # Release build optimizations
    target_compile_options(bbs-terminal PRIVATE
        $<$<CONFIG:Release>:/O2>    # Optimize for speed
        $<$<CONFIG:Release>:/Oi>    # Intrinsic functions
    )
else()
    target_compile_options(bbs-terminal PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -std=c++20
    )
endif()

# Set output directory
set_target_properties(bbs-terminal PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

# Version information
set_target_properties(bbs-terminal PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)

# Print configuration
message(STATUS "BBS Terminal Emulator - Configuration Summary")
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")
message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "TARGET: bbs-terminal")
message(STATUS "VERSION: ${PROJECT_VERSION}")
