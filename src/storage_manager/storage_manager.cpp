//
// Created by marce on 4/23/2025.
//

#include "storage_manager.h"

#include <filesystem>

StorageManager::StorageManager():
    directory_path_("files"),
    db_file_("db.json") {
}

StorageManager::~StorageManager() {
}

#include <fstream>

void StorageManager::init() {
    if (!std::filesystem::exists(directory_path_)) {
        std::filesystem::create_directory(directory_path_);
    }
    if (!std::filesystem::exists(directory_path_ / db_file_)) {
        std::ofstream db_file(directory_path_ / db_file_);
    }




}

void StorageManager::load_file() {
}

void StorageManager::save_file(const std::string& name, const std::vector<std::byte>& data) {
}