cmake_minimum_required(VERSION 3.20)
project(BBSTerminalEmulator VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Detect platform
if(CMAKE_GENERATOR MATCHES "Win32")
    set(PLATFORM_NAME "Win32")
    set(IS_32BIT TRUE)
elseif(CMAKE_GENERATOR MATCHES "x64" OR CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(PLATFORM_NAME "x64")
    set(IS_32BIT FALSE)
else()
    set(PLATFORM_NAME "x64")
    set(IS_32BIT FALSE)
endif()

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

# Platform-specific settings
if(IS_32BIT)
    set_target_properties(bbs-terminal PROPERTIES
        SUFFIX ".exe"
        PREFIX ""
    )
    # 32-bit specific compile options
    if(MSVC)
        target_compile_options(bbs-terminal PRIVATE
            /arch:SSE2  # SSE2 for 32-bit
        )
    endif()
else()
    set_target_properties(bbs-terminal PROPERTIES
        SUFFIX ".exe"
        PREFIX ""
    )
    # 64-bit specific compile options
    if(MSVC)
        target_compile_options(bbs-terminal PRIVATE
            /arch:AVX2  # AVX2 for 64-bit
        )
    endif()
endif()

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
    
    # Debug build options
    target_compile_options(bbs-terminal PRIVATE
        $<$<CONFIG:Debug>:/Zi>      # Program database
        $<$<CONFIG:Debug>:/RTC1>    # Runtime checks
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
if(IS_32BIT)
    set_target_properties(bbs-terminal PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/Win32
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/Win32
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/Win32
    )
else()
    set_target_properties(bbs-terminal PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/x64
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/x64
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/x64
    )
endif()

# Version information
set_target_properties(bbs-terminal PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)

# Define platform macro
if(IS_32BIT)
    target_compile_definitions(bbs-terminal PRIVATE
        PLATFORM_WIN32
        _WIN32
    )
else()
    target_compile_definitions(bbs-terminal PRIVATE
        PLATFORM_X64
        _WIN64
    )
endif()

# Print configuration
message(STATUS "BBS Terminal Emulator - Configuration Summary")
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")
message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "PLATFORM: ${PLATFORM_NAME}")
message(STATUS "TARGET: bbs-terminal")
message(STATUS "VERSION: ${PROJECT_VERSION}")
if(IS_32BIT)
    message(STATUS "BUILD TYPE: 32-bit (Win32)")
else()
    message(STATUS "BUILD TYPE: 64-bit (x64)")
endif()
