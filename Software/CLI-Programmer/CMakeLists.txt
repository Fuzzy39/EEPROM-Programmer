cmake_minimum_required(VERSION 3.28)

project(CLI-Progammer LANGUAGES CXX)


add_executable(CLI-Programmer "src/main.cpp" "src/usb.cpp" "src/programmerBasic.cpp" "src/Comms.cpp")
target_include_directories(CLI-Programmer PUBLIC "src/include")
# oh boy ij ofeougrjfjm

IF(WIN32)
    add_library(usb-1.0 SHARED IMPORTED GLOBAL)
    set_target_properties(
        usb-1.0 
        PROPERTIES 
            IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/lib/libusb/win/libusb-1.0.dll
            IMPORTED_IMPLIB   ${PROJECT_SOURCE_DIR}/lib/libusb/win/libusb-1.0.lib
    ) 
    target_link_libraries(CLI-Programmer usb-1.0)

    # Cmake eludes me, I have no idea how you're supposed to use it. Copy the dang dll.
    add_custom_command(TARGET CLI-Programmer POST_BUILD        
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        ${PROJECT_SOURCE_DIR}/lib/libusb/win/libusb-1.0.dll
        $<TARGET_FILE_DIR:CLI-Programmer>)  

ELSE()
    target_link_libraries(CLI-Programmer usb-1.0)
ENDIF()