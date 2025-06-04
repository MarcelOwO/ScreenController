add_library(common INTERFACE
        command.h
        file_type.h
        pixel_data.h
        bluetooth_packet.h
)

target_include_directories(common
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:.>
)