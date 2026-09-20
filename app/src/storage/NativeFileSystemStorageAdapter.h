#pragma once

#include <string>
#include <vector>
#include <optional>
#include <cstdint>
#include <filesystem>

namespace catchim::storage {

class NativeFileSystemStorageAdapter {
public:
    explicit NativeFileSystemStorageAdapter(std::string directoryName = "media");

    const std::string& directoryName() const noexcept { return directoryName_; }
    const std::filesystem::path& rootPath() const noexcept { return rootPath_; }

    void setRootPath(const std::filesystem::path& path);

    std::optional<std::vector<uint8_t>> get(const std::string& key) const;

    bool set(const std::string& key, const std::vector<uint8_t>& data);
    bool set(const std::string& key, const uint8_t* data, size_t size);

    bool remove(const std::string& key);

    std::vector<std::string> list() const;

    bool clear();

    bool exists(const std::string& key) const;

    uint64_t size(const std::string& key) const;

private:
    std::string directoryName_;
    std::filesystem::path rootPath_;

    std::filesystem::path resolveKeyPath(const std::string& key) const;
};

} // namespace catchim::storage
