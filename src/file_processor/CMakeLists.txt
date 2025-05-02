add_library(File STATIC file_processor.cpp file_processor.h)

find_package(PkgConfig REQUIRED)

pkg_check_modules(LIBAV REQUIRED
        libavdevice
        libavfilter
        libavformat
        libavcodec
        libswresample
        libswscale
        libavutil
)

pkg_check_modules(webp REQUIRED libwebp)

target_include_directories(File PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${LIBAV_INCLUDE_DIRS}
        ${webp_INCLUDE_DIRS}
)

target_link_libraries(File
        ${LIBAV_LIBRARIES}
        ${webp_LIBRARIES}
)

link_directories(${LIBAV_LIBRARY_DIRS})

target_compile_options(File PUBLIC ${LIBAV_CFLAGS_OTHER})
