cmake_minimum_required(VERSION 3.20)
project(BBSTerminalEmulator)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Windows-specific configuration
if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /permissive- /std:c++latest")
endif()

# Main executable
add_executable(bbs-terminal
    src/main.cpp
    src/telnet/telnet_client.cpp
    src/terminal/ansi_interpreter.cpp
    src/terminal/rip_interpreter.cpp
    src/terminal/screen_buffer.cpp
    src/ui/main_window.cpp
    src/ui/terminal_view.cpp
    src/dialing/phonebook.cpp
    src/config/settings.cpp
)

target_include_directories(bbs-terminal PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Windows specific libraries
if(WIN32)
    target_link_libraries(bbs-terminal
        ws2_32
        wininet
        user32
        gdi32
        kernel32
    )
endif()
