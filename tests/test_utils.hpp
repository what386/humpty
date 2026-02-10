#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace humpty::tests {

inline std::vector<std::byte> make_test_data(std::size_t size) {
    std::vector<std::byte> data(size);
    std::uint32_t state = 0x12345678U;
    for (std::size_t i = 0; i < size; ++i) {
        state = (state * 1664525U) + 1013904223U;
        data[i] = static_cast<std::byte>(state & 0xFFU);
    }
    return data;
}

inline bool write_bytes(const std::filesystem::path& path, const std::vector<std::byte>& data, std::string& error) {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out.is_open()) {
        error = "Failed to open file for write: " + path.string();
        return false;
    }
    if (!data.empty()) {
        out.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
    }
    if (!out.good()) {
        error = "Failed writing file: " + path.string();
        return false;
    }
    return true;
}

inline bool read_bytes(const std::filesystem::path& path, std::vector<std::byte>& data, std::string& error) {
    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) {
        error = "Failed to open file for read: " + path.string();
        return false;
    }

    in.seekg(0, std::ios::end);
    const auto end_pos = in.tellg();
    if (end_pos < 0) {
        error = "Failed to seek file: " + path.string();
        return false;
    }

    const auto size = static_cast<std::size_t>(end_pos);
    data.assign(size, std::byte{0});
    in.seekg(0, std::ios::beg);

    if (size > 0) {
        in.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(size));
    }
    if (!in.good() && !in.eof()) {
        error = "Failed reading file: " + path.string();
        return false;
    }

    return true;
}

inline std::filesystem::path make_temp_dir(const std::string& name) {
    const auto base = std::filesystem::temp_directory_path();
    const auto dir = base / ("humpty-tests-" + name);
    std::error_code ec;
    std::filesystem::remove_all(dir, ec);
    std::filesystem::create_directories(dir, ec);
    return dir;
}

inline bool check(bool condition, const std::string& message, std::string& error) {
    if (!condition) {
        error = message;
        return false;
    }
    return true;
}

}  // namespace humpty::tests
