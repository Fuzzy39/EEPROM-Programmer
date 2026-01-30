cmake_minimum_required(VERSION 3.28)

project(CLI-Progammer LANGUAGES CXX)


add_executable(CLI-Programmer "src/main.cpp" "src/usb.cpp" "src/programmerBasic.cpp" "src/Comms.cpp")
target_include_directories(CLI-Programmer PUBLIC "src/headers")
target_link_libraries(CLI-Programmer usb-1.0)