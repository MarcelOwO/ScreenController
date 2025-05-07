find_package(PkgConfig REQUIRED)

pkg_check_modules(AVFORMAT REQUIRED libavformat)
pkg_check_modules(AVCODEC REQUIRED libavcodec)
pkg_check_modules(AVUTIL REQUIRED libavutil)
pkg_check_modules(SWSCALE REQUIRED libswscale)

pkg_check_modules(WEBP REQUIRED libwebp
)
add_library(File STATIC file_processor.cpp file_processor.h
        decoders/video_decoder.cpp
        decoders/video_decoder.h
        decoders/stb_decoder.cpp
        decoders/stb_decoder.h
        decoders/webp_decoder.cpp
        decoders/webp_decoder.h
        decoders/decoder.h
        models/frame_data.h
        decoders/decoder_factory.cpp
        decoders/decoder_factory.h)


include_directories(/usr/include/aarch64-linux-gnu/libavcodec)
include_directories(/usr/include/aarch64-linux-gnu/libavdevice)
include_directories(/usr/include/aarch64-linux-gnu/libavutil)
include_directories(/usr/include/aarch64-linux-gnu/libavformat)
include_directories(/usr/include/aarch64-linux-gnu/libswscale)
include_directories(/usr/include/webp)

target_include_directories(File PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${AVFORMAT_INCLUDE_DIRS}
        ${AVCODEC_INCLUDE_DIRS}
        ${AVUTIL_INCLUDE_DIRS}
        ${SWSCALE_INCLUDE_DIRS}
        ${WEBP_INCLUDE_DIRS}
)

target_link_directories(File PUBLIC
        ${AVFORMAT_LIBRARY_DIRS}
        ${AVCODEC_LIBRARY_DIRS}
        ${AVUTIL_LIBRARY_DIRS}
        ${SWSCALE_LIBRARY_DIRS}
        ${WEBP_LIBRARY_DIRS}
)

target_link_libraries(File
        ${AVFORMAT_LIBRARIES}
        ${AVCODEC_LIBRARIES}
        ${AVUTIL_LIBRARIES}
        ${SWSCALE_LIBRARIES}
        ${WEBP_LIBRARIES}
)