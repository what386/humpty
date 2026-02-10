#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "models/chunk.hpp"

namespace humpty::models {

struct Manifest {
    std::string format_version = "1";
    std::string source_file_name;
    std::uint64_t source_size = 0;
    std::uint64_t chunk_size = 0;
    std::string source_checksum;
    std::vector<Chunk> chunks;

    [[nodiscard]] bool is_valid() const;
};

bool write_manifest(const Manifest& manifest, const std::filesystem::path& path, std::string& error);
std::optional<Manifest> read_manifest(const std::filesystem::path& path, std::string& error);

}  // namespace humpty::models
