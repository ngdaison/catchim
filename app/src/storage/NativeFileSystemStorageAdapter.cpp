#include "NativeFileSystemStorageAdapter.h"
#include <fstream>
#include <system_error>

namespace catchim::storage {

NativeFileSystemStorageAdapter::NativeFileSystemStorageAdapter(std::string directoryName)
    : directoryName_(std::move(directoryName)) {
    rootPath_ = std::filesystem::temp_directory_path() / "catchim_storage" / directoryName_;
    std::error_code ec;
    std::filesystem::create_directories(rootPath_, ec);
}

void NativeFileSystemStorageAdapter::setRootPath(const std::filesystem::path& path) {
    rootPath_ = path;
    std::error_code ec;
    std::filesystem::create_directories(rootPath_, ec);
}

std::filesystem::path NativeFileSystemStorageAdapter::resolveKeyPath(const std::string& key) const {
    return rootPath_ / key;
}

std::optional<std::vector<uint8_t>> NativeFileSystemStorageAdapter::get(const std::string& key) const {
    auto path = resolveKeyPath(key);
    std::error_code ec;
    if (!std::filesystem::exists(path, ec) || !std::filesystem::is_regular_file(path, ec)) {
        return std::nullopt;
    }

    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return std::nullopt;
    }

    auto fileSize = file.tellg();
    if (fileSize < 0) {
        return std::nullopt;
    }

    std::vector<uint8_t> buffer(static_cast<size_t>(fileSize));
    file.seekg(0, std::ios::beg);
    if (fileSize > 0) {
        file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
    }
    return buffer;
}

bool NativeFileSystemStorageAdapter::set(const std::string& key, const std::vector<uint8_t>& data) {
    return set(key, data.data(), data.size());
}

bool NativeFileSystemStorageAdapter::set(const std::string& key, const uint8_t* data, size_t size) {
    auto path = resolveKeyPath(key);
    std::error_code ec;
    std::filesystem::create_directories(path.parent_path(), ec);

    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file.is_open()) {
        return false;
    }

    if (size > 0 && data != nullptr) {
        file.write(reinterpret_cast<const char*>(data), static_cast<std::streamsize>(size));
    }
    file.flush();
    return file.good();
}

bool NativeFileSystemStorageAdapter::remove(const std::string& key) {
    auto path = resolveKeyPath(key);
    std::error_code ec;
    return std::filesystem::remove(path, ec);
}

std::vector<std::string> NativeFileSystemStorageAdapter::list() const {
    std::vector<std::string> keys;
    std::error_code ec;
    if (!std::filesystem::exists(rootPath_, ec)) {
        return keys;
    }

    for (const auto& entry : std::filesystem::directory_iterator(rootPath_, ec)) {
        if (entry.is_regular_file(ec)) {
            keys.push_back(entry.path().filename().string());
        }
    }
    return keys;
}

bool NativeFileSystemStorageAdapter::clear() {
    std::error_code ec;
    if (!std::filesystem::exists(rootPath_, ec)) {
        return true;
    }

    for (const auto& entry : std::filesystem::directory_iterator(rootPath_, ec)) {
        std::filesystem::remove_all(entry.path(), ec);
    }
    return true;
}

bool NativeFileSystemStorageAdapter::exists(const std::string& key) const {
    auto path = resolveKeyPath(key);
    std::error_code ec;
    return std::filesystem::exists(path, ec);
}

uint64_t NativeFileSystemStorageAdapter::size(const std::string& key) const {
    auto path = resolveKeyPath(key);
    std::error_code ec;
    if (!std::filesystem::exists(path, ec)) {
        return 0;
    }
    return static_cast<uint64_t>(std::filesystem::file_size(path, ec));
}

} // namespace catchim::storage
