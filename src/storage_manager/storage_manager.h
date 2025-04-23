//
// Created by marce on 4/23/2025.
//

#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include <filesystem>
#include <string>
#include <vector>

class StorageManager {
public:
    StorageManager();
    ~StorageManager();

    void init();
    void run();
    void cleanup();
    void load_file();
    void save_file(const std::string& name, const std::vector<std::byte>& data);

private:
    std::filesystem::path directory_path_;
    std::filesystem::path db_file_;
};


#endif //STORAGE_MANAGER_H
