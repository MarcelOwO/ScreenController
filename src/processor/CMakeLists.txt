find_package(PkgConfig REQUIRED)

pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET
  libavformat
  libavcodec
  libavutil
  libswscale)
find_path(WEBP_INCLUDE_DIR webp/decode.h REQUIRED)
find_library(WEBP_LIBRARY webp REQUIRED)

add_library(File STATIC
  src/file_processor_impl.cpp
  src/file_processor_factory.cpp
  src/decoders/video_decoder.cpp
  src/decoders/stb_decoder.cpp
  src/decoders/webp_decoder.cpp
  src/decoders/decoder_factory.cpp
)

target_include_directories(File
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        PRIVATE
        ${WEBP_INCLUDE_DIR}
)

target_link_libraries(File
        PUBLIC
        Logger common
        PRIVATE
        PkgConfig::FFMPEG
        ${WEBP_LIBRARY}
)
