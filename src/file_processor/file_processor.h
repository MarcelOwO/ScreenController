//
// Created by marce on 4/23/2025.
//

#ifndef FILE_PROCESSOR_H
#define FILE_PROCESSOR_H

#include <array>
#include <optional>

#include "../common/pixel_data.h"

class FileProcessor {
public:
    FileProcessor();
    ~FileProcessor();
    void init();
    void run();
    void cleanup();

    std::optional<std::array<PixelData, static_cast<std::size_t>(1920 * 1080)>> next_frame();
};


#endif //FILE_PROCESSOR_H
