find_package(PkgConfig REQUIRED)

pkg_check_modules(AVFORMAT REQUIRED libavformat)
pkg_check_modules(AVCODEC REQUIRED libavcodec)
pkg_check_modules(AVUTIL REQUIRED libavutil)
pkg_check_modules(SWSCALE REQUIRED libswscale)

pkg_check_modules(WEBP REQUIRED libwebp
)
<<<<<<<< HEAD:source/file_processor/CMakeLists.txt
add_library(File STATIC
        file_processor.cpp
        decoders/video_decoder.cpp
        decoders/stb_decoder.cpp
        decoders/webp_decoder.cpp
        decoders/decoder_factory.cpp
)

target_include_directories(File
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/.
========

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
>>>>>>>> origin/dev:source/processor/CMakeLists.txt
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
        PUBLIC
        Logger common
        PRIVATE
        ${AVFORMAT_LIBRARIES}
        ${AVCODEC_LIBRARIES}
        ${AVUTIL_LIBRARIES}
        ${SWSCALE_LIBRARIES}
        ${WEBP_LIBRARIES}
<<<<<<<< HEAD:source/file_processor/CMakeLists.txt

)
========
)
>>>>>>>> origin/dev:source/processor/CMakeLists.txt
