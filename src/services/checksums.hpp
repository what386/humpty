#pragma once

#include <cstdint>
#include <filesystem>
#include <span>
#include <string>

namespace humpty::services {

std::uint64_t fnv1a64(std::span<const std::byte> data, std::uint64_t seed = 14695981039346656037ULL);
std::string fnv1a64_hex(std::span<const std::byte> data, std::uint64_t seed = 14695981039346656037ULL);

bool hash_file_fnv1a64_hex(const std::filesystem::path& path, std::string& out_hex, std::string& error);

}  // namespace humpty::services
